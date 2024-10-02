#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <iostream>
#include <string>
#include <vector>

#include "Utils.hpp"
#include "HttpServer.hpp"
#include "RedisPeerStorage.hpp"
#include "FileMetadata.hpp"
#include "PeerInfo.hpp"

HttpServer::HttpServer(net::io_context& ioc, tcp::acceptor& acceptor, RedisPeerStorage& redisDb)
    : m_ioc(ioc), m_acceptor(acceptor), m_redisDb(redisDb) {
}

void HttpServer::start() {
    doAccept();
}

void HttpServer::doAccept() {
    m_acceptor.async_accept(net::make_strand(m_ioc), [this](boost::system::error_code ec, tcp::socket socket) {
        auto socket_ptr = std::make_unique<tcp::socket>(std::move(socket));
        onAccept(ec, std::move(socket_ptr));
    });
}

void HttpServer::onAccept(boost::system::error_code ec, std::unique_ptr<tcp::socket> socket) {
    if (!ec) {
        auto buffer = std::make_shared<beast::flat_buffer>();
        auto request = std::make_shared<http::request<http::string_body>>();
        http::async_read(
            *socket, *buffer, *request,
            [this, request, buffer, socket = std::move(socket)](boost::system::error_code ec, std::size_t) mutable {
                if (!ec) {
                    processRequest(buffer, request, std::move(socket));
                } else {
                    std::cerr << "Failed to handle request: " << ec.message() << std::endl;
                }
            });
    } else {
        std::cout << "Failed to accept HTTP connection! Error: " << ec.message() << std::endl;
    }

    doAccept();
}

boost::json::object HttpServer::parseRequest(std::shared_ptr<http::request<http::string_body>> request) {
    boost::json::value json_value = boost::json::parse(request->body());
    return json_value.as_object();
}

void HttpServer::sendJsonResponse(const boost::json::object& responseJson, http::status status,
                                  unsigned int httpVersion, std::unique_ptr<tcp::socket> socket) {
    auto res = std::make_shared<http::response<http::string_body>>(status, httpVersion);
    res->set(http::field::server, "P2P File Sharing Server");
    res->set(http::field::content_type, "application/json");
    res->keep_alive(false);
    res->body() = boost::json::serialize(responseJson);
    res->prepare_payload();

    http::async_write(*socket, *res, [res, socket = std::move(socket)](beast::error_code ec, std::size_t) {
        if (ec) {
            std::cerr << "Error sending response: " << ec.message() << std::endl;
        }
    });
}

void HttpServer::handleDiscoveryRequest(const std::shared_ptr<http::request<http::string_body>>& request,
                                        std::unique_ptr<tcp::socket> socket) {
    auto reqJson = parseRequest(request);
    auto uuid = utils::generateUuid();
    json::object responseJson;
    responseJson["uuid"] = uuid;
    PeerInfo peer = {.peerUuid = std::move(uuid),
                     .peerIp = utils::getFieldValue<std::string>(reqJson, "peer_ip"),
                     .peerPort = utils::getFieldValue<uint64_t>(reqJson, "peer_port")};
    http::status status = (m_redisDb.storePeerInfo(peer)) ? http::status::ok : http::status::internal_server_error;
    sendJsonResponse(responseJson, status, request->version(), std::move(socket));
}

void HttpServer::handleFileAdvertisement(const std::shared_ptr<http::request<http::string_body>>& request,
                                         std::unique_ptr<tcp::socket> socket) {
    auto reqJson = parseRequest(request);
    std::vector<std::string> chunkHashes;
    for (const auto& chunkHash : reqJson.at("chunk_hashes").as_array()) {
        chunkHashes.push_back(chunkHash.as_string().c_str());
    }
    FileMetadata metaData = {.peerUuid = utils::getFieldValue<std::string>(reqJson, "peer_uuid"),
                             .fileName = utils::getFieldValue<std::string>(reqJson, "file_name"),
                             .fileNameUuid = utils::getFieldValue<std::string>(reqJson, "file_name_uuid"),
                             .fileDescription = utils::getFieldValue<std::string>(reqJson, "file_description"),
                             .fileSize = utils::getFieldValue<uint64_t>(reqJson, "file_size"),
                             .totalChunks = utils::getFieldValue<uint64_t>(reqJson, "total_chunks"),
                             .chunkHashes = std::move(chunkHashes)};
    http::status status =
        (m_redisDb.storeFileMetadata(metaData)) ? http::status::ok : http::status::internal_server_error;
    sendJsonResponse({}, status, request->version(), std::move(socket));
}

void HttpServer::handleFileListRequest(const std::shared_ptr<http::request<http::string_body>>& request,
                                       std::unique_ptr<tcp::socket> socket) {
    auto fileList = m_redisDb.retrieveAllFileDetails();
    json::object responseJson;
    for (const auto& file : fileList) {
        json::object fileJson;
        fileJson["file_name"] = std::move(file.fileName);
        fileJson["file_description"] = std::move(file.fileDescription);
        fileJson["file_size"] = file.fileSize;
        responseJson[std::move(file.fileNameUuid)] = std::move(fileJson);
    }
    sendJsonResponse(responseJson, http::status::ok, request->version(), std::move(socket));
}

void HttpServer::handleFileRequest(const std::shared_ptr<http::request<http::string_body>>& request,
                                   std::unique_ptr<tcp::socket> socket) {
    auto reqJson = parseRequest(request);
    auto fileUuid = utils::getFieldValue<std::string>(reqJson, "file_uuid");
    auto fileDetails = m_redisDb.retrieveFileDetails(fileUuid);
    if (!fileDetails.has_value()) {
        std::cout << "Failed to retrieve file details" << std::endl;
        return;
    }
    json::object responseJson;
    json::array chunkArray;
    for (const auto& chunk : (*fileDetails).chunkDetails) {
        json::object chunkJson;
        chunkJson["hash"] = std::move(chunk.hash);
        json::array peerArray;
        for (const auto& peer : chunk.peers) {
            json::object peerJson;
            peerJson["ip"] = peer.ip;
            peerJson["port"] = peer.port;
            peerArray.push_back(std::move(peerJson));
        }
        chunkJson["peers"] = std::move(peerArray);
        chunkArray.push_back(std::move(chunkJson));
    }
    responseJson["chunks"] = std::move(chunkArray);
    sendJsonResponse(responseJson, http::status::ok, request->version(), std::move(socket));
}

void HttpServer::processRequest(std::shared_ptr<beast::flat_buffer> buffer,
                                std::shared_ptr<http::request<http::string_body>> request,
                                std::unique_ptr<tcp::socket> socket) {
    try {
        if (request->method() == http::verb::post) {
            auto target = request->target();
            if (target == "/discovery") {
                handleDiscoveryRequest(request, std::move(socket));
            } else if (target == "/file_advert") {
                handleFileAdvertisement(request, std::move(socket));
            } else {
                std::cerr << "Unsupported target. Dropping HTTP request" << std::endl;
            }
        } else if (request->method() == http::verb::get) {
            auto target = request->target();
            if (target == "/file_list") {
                handleFileListRequest(request, std::move(socket));
            } else if (target == "/file_request") {
                handleFileRequest(request, std::move(socket));
            }
        }
    } catch (const boost::json::system_error& e) {
        std::cerr << "JSON parsing failed: " << e.what() << std::endl;
    } catch (const std::runtime_error& e) {
        std::cerr << "Failed to extract field from JSON " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
    }
}