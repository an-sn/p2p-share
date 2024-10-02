#pragma once

#include <string>
#include <vector>
#include <utility>

struct IpPortPair {
    std::string ip;
    std::string port;
};

using PeerList = std::vector<IpPortPair>;

struct ChunkInfo {
    std::string hash;
    PeerList peers;
};

struct FileMetadata {
    std::string peerUuid;
    std::string fileName;
    std::string fileNameUuid;
    std::string fileDescription;
    uint64_t fileSize;
    uint64_t totalChunks;
    std::vector<std::string> chunkHashes;
    std::vector<ChunkInfo> chunkDetails;
};