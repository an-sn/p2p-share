#pragma once

#include "PeerInfo.hpp"

class IPeerStorage
{
    public:
        virtual ~IPeerStorage() = default;

        virtual void
        addPeer(const boost::asio::ip::tcp::endpoint &endpoint) = 0;
        virtual const PeerInfo &
        getPeer(const boost::asio::ip::tcp::endpoint &endpoint) const = 0;
};
