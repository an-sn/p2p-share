#include "RedisPeerStorage.hpp"
#include <hiredis/hiredis.h>
#include <stdexcept>

RedisPeerStorage::RedisPeerStorage()
{
}

RedisPeerStorage::~RedisPeerStorage()
{
    if (m_redisContext)
    {
        redisFree(m_redisContext);
    }
}

void RedisPeerStorage::addPeer(const boost::asio::ip::tcp::endpoint &endpoint)
{
}
const PeerInfo &
RedisPeerStorage::getPeer(const boost::asio::ip::tcp::endpoint &endpoint) const
{
}

bool RedisPeerStorage::connect(std::string ipAddress, unsigned short port)
{
    m_redisContext = redisConnect(ipAddress.c_str(), port);
    if (m_redisContext == nullptr)
    {
        return false;
    }
    if (m_redisContext->err)
    {
        std::string errorMessage = "Failed to connect to Redis: ";
        errorMessage += m_redisContext->errstr;
        redisFree(m_redisContext);
        m_redisContext = nullptr;
        std::cout << errorMessage << std::endl;
        return false;
    }
    std::cout << "Successfully connected to Redis DB!" << std::endl;
    return true;
}