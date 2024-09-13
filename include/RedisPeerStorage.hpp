#ifndef REDIS_PEER_STORAGE_HPP
#define REDIS_PEER_STORAGE_HPP

#include <hiredis/hiredis.h>
#include <vector>
#include <string>

#include "PeerStorage.hpp"

class RedisPeerStorage : public IPeerStorage {
public:
    RedisPeerStorage();
    RedisPeerStorage(std::string ipAddress, unsigned short port);
    ~RedisPeerStorage();

    void addPeer(const boost::asio::ip::tcp::endpoint& endpoint) override;
    const PeerInfo& getPeer(const boost::asio::ip::tcp::endpoint& endpoint) const override;
    bool connectToRedisDb();

private:
    // std::string endpointToPeerId(const boost::asio::ip::tcp::endpoint& endpoint) const;
    // std::string getPeerId(const std::string& endpoint) const;
    redisContext* m_redisContext;
    std::string m_ipAddress;
    unsigned short m_port;
};

#endif // REDIS_PEER_STORAGE_HPP