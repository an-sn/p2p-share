#pragma once

#include <string>
#include <vector>
#include <utility>

#include "PeerInfo.hpp"

struct ChunkAdvertisement {
    std::string fileUuid;
    std::string peerUuid;
    uint64_t chunkId;
};

using PeerList = std::vector<PeerInfo>;

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