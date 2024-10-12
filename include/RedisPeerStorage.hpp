#pragma once

#include <hiredis/hiredis.h>

#include <string>
#include <vector>
#include <optional>

struct PeerInfo;
struct FileMetadata;
struct ChunkAdvertisement;

class RedisPeerStorage {
  public:
    RedisPeerStorage();
    ~RedisPeerStorage();

    bool connect(std::string ipAddress, unsigned short port);
    bool storePeerInfo(const PeerInfo& peerInfo);
    bool storeFileMetadata(const FileMetadata& fileMetadata);
    bool updateChunkPeerList(const ChunkAdvertisement& chunkAdvert);
    std::vector<FileMetadata> retrieveAllFileDetails();
    std::optional<FileMetadata> retrieveFileDetails(const std::string& uuid);

  private:
    redisContext* m_redisContext;
};