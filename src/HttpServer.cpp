#include "HttpServer.hpp"

#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/json.hpp>
#include <iostream>
#include <string>

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
            [this, request, buffer, socket = std::move(socket)](boost::system::error_code ec, std::size_t) {
                if (!ec) {
                    processRequest(buffer, request);
                } else {
                    std::cerr << "Failed to handle request: " << ec.message() << std::endl;
                }
            });
    } else {
        std::cout << "Failed to accept HTTP connection! Error: " << ec.message() << std::endl;
    }

    doAccept();
}

void HttpServer::processRequest(std::shared_ptr<beast::flat_buffer> buffer,
                                std::shared_ptr<http::request<http::string_body>> request) {
    std::cout << "Request Method: " << request->method_string() << std::endl;
    std::cout << "Request Target: " << request->target() << std::endl;

    // Print headers
    std::cout << "Request Headers:" << std::endl;
    for (const auto& field : request->base()) {
        std::cout << field.name_string() << ": " << field.value() << std::endl;
    }

    // Parse the request body if it's a POST or PUT
    if (request->method() == http::verb::post || request->method() == http::verb::put) {
        try {
            // Parse the request body as JSON
            boost::json::value json_value = boost::json::parse(request->body());

            // Extract JSON fields (assuming they exist)
            boost::json::object json_obj = json_value.as_object();
            std::string peer_ip = json_obj["peer_ip"].as_string().c_str();
            int peer_port = json_obj["peer_port"].as_int64();
            std::string name = json_obj["name"].as_string().c_str();

            // Print extracted fields
            std::cout << "Parsed JSON:" << std::endl;
            std::cout << "Peer IP: " << peer_ip << std::endl;
            std::cout << "Peer Port: " << peer_port << std::endl;
            std::cout << "Name: " << name << std::endl;
        } catch (const boost::json::system_error& e) {
            std::cerr << "JSON parsing failed: " << e.what() << std::endl;
        }
    }
}