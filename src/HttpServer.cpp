#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <iostream>
#include <string>

#include "Utils.hpp"
#include "HttpServer.hpp"

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

boost::json::object HttpServer::parseRequest(std::shared_ptr<http::request<http::string_body>> request)
{
    boost::json::value json_value = boost::json::parse(request->body());
    boost::json::object json_obj = json_value.as_object();
    return json_obj;
}

void HttpServer::processRequest(std::shared_ptr<beast::flat_buffer> buffer,
                                std::shared_ptr<http::request<http::string_body>> request,
                                std::unique_ptr<tcp::socket> socket) {
    std::cout << "Request Method: " << request->method_string() << std::endl;
    std::cout << "Request Target: " << request->target() << std::endl;

    std::cout << "Request Headers:" << std::endl;
    for (const auto& field : request->base()) {
        std::cout << field.name_string() << ": " << field.value() << std::endl;
    }

    if (request->method() == http::verb::post || request->method() == http::verb::put) {
        try {
            auto reqJson = parseRequest(request);

            switch(request->target())
            {
                case "discovery":
                    
                default:
                    std::cerr << "Unsupported target. Dropping HTTP request" << std::endl;
            }

            auto peerId = utils::getFieldValue<std::string>(reqJson, "peer_ip");
            auto peerPort = utils::getFieldValue<int64_t>(reqJson, "peer_port");

            // std::cout << "Peer IP: " << peerId << std::endl;
            // std::cout << "Peer Port: " << peerPort << std::endl;
            auto uuid = utils::generateUuid();

            json::object responseJson;
            responseJson["uuid"] = uuid;
            
            auto res = std::make_shared<http::response<http::string_body>>(http::status::ok, request->version());

            // http::response<http::string_body> res{http::status::ok, request->version()};
            res->set(http::field::server, "P2P File Sharing Server");
            res->set(http::field::content_type, "application/json");
            res->keep_alive(request->keep_alive());
            res->body() = json::serialize(responseJson);
            res->prepare_payload();
            http::async_write(*socket, *res,
                [this, res, socket = std::move(socket)](beast::error_code ec, std::size_t) {
                    if (ec) {
                        std::cerr << "Error sending response: " << ec.message() << std::endl;
                    }
                });

            // std::cout << "UUID : " << uuid << std::endl;

        } catch (const boost::json::system_error& e) {
            std::cerr << "JSON parsing failed: " << e.what() << std::endl;
        } catch (const std::runtime_error& e) {
            std::cerr << "Failed to extract field from JSON " << e.what() << std::endl; 
        }
    }
}