#pragma once

#include <string>
#include <vector>

struct FileMetadata {
    std::string peerUuid;
    std::string fileName;
    std::string fileNameUuid;
    std::string fileDescription;
    uint64_t fileSize;
    uint64_t totalChunks;
    std::vector<std::string> chunkHashes;
};