#pragma once

#include <boost/asio/ip/tcp.hpp>

struct PeerInfo {
    boost::asio::ip::tcp::endpoint endpoint;

    PeerInfo(const boost::asio::ip::tcp::endpoint& ep = boost::asio::ip::tcp::endpoint())
        : endpoint(ep) {}
};
