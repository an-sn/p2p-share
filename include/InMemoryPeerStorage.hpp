#pragma once

#include "PeerStorage.hpp"
#include <unordered_map>

class InMemoryPeerStorage : public IPeerStorage {
public:
    void addPeer(const boost::asio::ip::tcp::endpoint& endpoint) override;
    const PeerInfo& getPeer(const boost::asio::ip::tcp::endpoint& endpoint) const override;

private:
    std::unordered_map<boost::asio::ip::tcp::endpoint, PeerInfo> m_peerList;
};
