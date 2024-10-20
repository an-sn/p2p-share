#pragma once

#include "HttpServer.hpp"
#include "RedisPeerStorage.hpp"

constexpr int PORT_NUMBER = 8080;

class P2PServer {
  public:
    P2PServer();
    ~P2PServer();
    void startListening(int threadCount);
    bool connectToDatabase(std::string ipAddress, unsigned short port);
    void stopServer();

  private:
    net::io_context m_ioc;
    tcp::acceptor m_acceptor;
    RedisPeerStorage m_redisDb;
};
