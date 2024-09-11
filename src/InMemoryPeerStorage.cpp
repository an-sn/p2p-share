#include "InMemoryPeerStorage.hpp"
#include <stdexcept>

void InMemoryPeerStorage::addPeer(const boost::asio::ip::tcp::endpoint& endpoint) {
    peerList_[endpoint] = PeerInfo(endpoint);
}

const PeerInfo& InMemoryPeerStorage::getPeer(const boost::asio::ip::tcp::endpoint& endpoint) const {
    auto it = peerList_.find(endpoint);
    if (it != peerList_.end()) {
        return it->second;
    }
    throw std::runtime_error("Peer not found");
}