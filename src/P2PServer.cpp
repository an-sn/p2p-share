#include "P2PServer.hpp"

#include <thread>
#include <vector>

P2PServer::P2PServer() : m_acceptor(m_ioc) {
}

P2PServer::~P2PServer() {
}

bool P2PServer::connectToDatabase(std::string ipAddress, unsigned short port) {
    return m_redisDb.connect(ipAddress, port);
}

void P2PServer::startListening(int threadCount) {
    tcp::endpoint endpoint(tcp::v4(), PORT_NUMBER);
    m_acceptor.open(endpoint.protocol());
    m_acceptor.set_option(tcp::acceptor::reuse_address(true));
    m_acceptor.bind(endpoint);
    m_acceptor.listen();

    HttpServer httpServer(m_ioc, m_acceptor, m_redisDb);
    httpServer.start();

    std::vector<std::thread> workerThreads;
    for (auto threadInd = 0; threadInd < threadCount; threadInd++) {
        workerThreads.push_back(std::thread([this]() { m_ioc.run(); }));
    }
    m_ioc.run();
    for (auto& thread : workerThreads) {
        thread.join();
    }
}

void P2PServer::stopServer() {
    m_redisDb.closeConnection();
    m_ioc.stop();
}