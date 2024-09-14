#include "HttpServer.hpp"
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/strand.hpp>
#include <iostream>

HttpServer::HttpServer(net::io_context& ioc, tcp::acceptor& acceptor)
    : m_ioc(ioc), m_acceptor(acceptor) {}

void HttpServer::start() {
    doAccept();
}

void HttpServer::doAccept() {
    m_acceptor.async_accept(
        net::make_strand(m_ioc),
        [this](boost::system::error_code ec, tcp::socket socket) {
            onAccept(ec, std::move(socket));
        });
}

void HttpServer::onAccept(boost::system::error_code ec, tcp::socket socket) {
    if (!ec) {
        std::cout << "Accepted connection from: " << socket.remote_endpoint() << std::endl;

        // Create a simple HTTP response
        auto const response = std::make_shared<http::response<http::string_body>>(http::status::ok, 11);
        response->set(http::field::server, "P2PFileSharingServer");
        response->set(http::field::content_type, "text/plain");
        response->body() = "Server is up";
        response->prepare_payload();

        // Send the HTTP response
        auto sp = std::make_shared<tcp::socket>(std::move(socket));
        http::async_write(*sp, *response,
            [sp, response](boost::system::error_code ec, std::size_t) {
                if (ec) {
                    std::cerr << "Error sending response: " << ec.message() << std::endl;
                }
                // Close the connection after sending the response
                sp->shutdown(tcp::socket::shutdown_send, ec);
            });
    }
    else{
        std::cout << "Error: " << ec.message() << std::endl;
    }

    // Accept the next connection
    doAccept();
}