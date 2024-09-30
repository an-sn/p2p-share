#pragma once

#include <string>
#include <vector>

struct PeerInfo {
    std::string peerUuid;
    std::string peerIp;
    uint64_t peerPort;
};