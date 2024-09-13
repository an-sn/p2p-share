#ifndef PEER_STORAGE_HPP
#define PEER_STORAGE_HPP

#include "PeerInfo.hpp"

class IPeerStorage {
public:
    virtual ~IPeerStorage() = default;

    virtual void addPeer(const boost::asio::ip::tcp::endpoint& endpoint) = 0;
    virtual const PeerInfo& getPeer(const boost::asio::ip::tcp::endpoint& endpoint) const = 0;
};

#endif // I_PEER_STORAGE_HPP