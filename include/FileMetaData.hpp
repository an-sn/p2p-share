#pragma once

#include <string>
#include <vector>

struct FileMetadata {
    std::string peerUuid;
    std::string fileName;
    std::string fileNameUuid;
    int64_t fileSize;
    int64_t totalChunks;
    std::vector<std::string> chunkHashes;
};