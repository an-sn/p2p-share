#pragma once

#include <hiredis/hiredis.h>

#include <string>
#include <vector>
#include <optional>

struct PeerInfo;
struct FileMetadata;

class RedisPeerStorage {
  public:
    RedisPeerStorage();
    ~RedisPeerStorage();

    bool connect(std::string ipAddress, unsigned short port);
    bool storePeerInfo(const PeerInfo& peerInfo);
    bool storeFileMetadata(const FileMetadata& fileMetadata);
    std::vector<FileMetadata> retrieveAllFileDetails();
    std::optional<FileMetadata> retrieveFileDetails(const std::string& uuid);

  private:
    redisContext* m_redisContext;
};