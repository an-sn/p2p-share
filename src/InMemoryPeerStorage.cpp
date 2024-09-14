#include "InMemoryPeerStorage.hpp"
#include <stdexcept>

void InMemoryPeerStorage::addPeer(
    const boost::asio::ip::tcp::endpoint &endpoint)
{
    m_peerList[endpoint] = PeerInfo(endpoint);
}

const PeerInfo &InMemoryPeerStorage::getPeer(
    const boost::asio::ip::tcp::endpoint &endpoint) const
{
    auto it = m_peerList.find(endpoint);
    if (it != m_peerList.end())
    {
        return it->second;
    }
    throw std::runtime_error("Peer not found");
}