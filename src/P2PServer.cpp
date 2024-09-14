#include "P2PServer.hpp"

P2PServer::P2PServer() : m_acceptor(m_ioc)
{
}

P2PServer::~P2PServer()
{
}

bool P2PServer::connectToDatabase(std::string ipAddress, unsigned short port)
{
    return m_redisDb.connect(ipAddress, port);
}

void P2PServer::startListening()
{
    tcp::endpoint endpoint(tcp::v4(), PORT_NUMBER);
    m_acceptor.open(endpoint.protocol());
    m_acceptor.set_option(tcp::acceptor::reuse_address(true));
    m_acceptor.bind(endpoint);
    m_acceptor.listen();

    HttpServer httpServer(m_ioc, m_acceptor);
    httpServer.start();
    m_ioc.run();
}