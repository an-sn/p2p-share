#ifndef IN_MEMORY_PEER_STORAGE_HPP
#define IN_MEMORY_PEER_STORAGE_HPP

#include "PeerStorage.hpp"
#include <unordered_map>
#include <boost/asio/ip/tcp.hpp>

class InMemoryPeerStorage : public IPeerStorage {
public:
    void addPeer(const boost::asio::ip::tcp::endpoint& endpoint) override;
    const PeerInfo& getPeer(const boost::asio::ip::tcp::endpoint& endpoint) const override;

private:
    std::unordered_map<boost::asio::ip::tcp::endpoint, PeerInfo> peerList_;
};

#endif // IN_MEMORY_PEER_STORAGE_HPP