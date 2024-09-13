#ifndef HTTP_SERVER_HPP
#define HTTP_SERVER_HPP

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <string>

namespace http = boost::beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

class HttpServer {
public:
    HttpServer(net::io_context& ioc, tcp::acceptor& acceptor);
    void start();

private:
    void doAccept();
    void onAccept(boost::system::error_code ec, tcp::socket socket);

    net::io_context& m_ioc;
    tcp::acceptor& m_acceptor;
};

#endif // HTTP_SERVER_HPP