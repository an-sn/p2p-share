#pragma once

#include <hiredis/hiredis.h>
#include <vector>
#include <string>
#include <iostream>

#include "PeerStorage.hpp"

class RedisPeerStorage : public IPeerStorage
{
    public:
        RedisPeerStorage();
        ~RedisPeerStorage();

        void addPeer(const boost::asio::ip::tcp::endpoint &endpoint) override;
        const PeerInfo &
        getPeer(const boost::asio::ip::tcp::endpoint &endpoint) const override;
        bool connect(std::string ipAddress, unsigned short port);

    private:
        redisContext  *m_redisContext;
        std::string    m_ipAddress;
        unsigned short m_port;
};
