#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/json.hpp>
#include <memory>

namespace beast = boost::beast;
namespace http = boost::beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

class RedisPeerStorage;

class HttpServer {
  public:
    HttpServer(net::io_context& ioc, tcp::acceptor& acceptor, RedisPeerStorage& redisDb);
    void start();

  private:
    void doAccept();
    void onAccept(boost::system::error_code ec, std::unique_ptr<tcp::socket> socket);
    void processRequest(std::shared_ptr<beast::flat_buffer> buffer,
                        std::shared_ptr<http::request<http::string_body>> request,
                        std::unique_ptr<tcp::socket> socket);
    boost::json::object parseRequest(std::shared_ptr<http::request<http::string_body>> request);
    net::io_context& m_ioc;
    tcp::acceptor& m_acceptor;
    RedisPeerStorage& m_redisDb;
};
