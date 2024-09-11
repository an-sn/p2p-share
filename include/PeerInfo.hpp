#ifndef PEER_INFO_HPP
#define PEER_INFO_HPP

#include <boost/asio/ip/tcp.hpp>

struct PeerInfo {
    boost::asio::ip::tcp::endpoint endpoint;

    PeerInfo(const boost::asio::ip::tcp::endpoint& ep = boost::asio::ip::tcp::endpoint())
        : endpoint(ep) {}
};

#endif // PEER_INFO_HPP