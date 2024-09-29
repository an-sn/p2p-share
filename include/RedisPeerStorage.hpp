#pragma once

#include <hiredis/hiredis.h>

#include <string>

struct PeerInfo;
struct FileMetadata;

class RedisPeerStorage {
  public:
    RedisPeerStorage();
    ~RedisPeerStorage();

    bool connect(std::string ipAddress, unsigned short port);
    bool storePeerInfo(const std::string& uuid, const std::string& peerIp, const std::string& peerPort);
    bool storeFileMetadata(FileMetadata &fileMetadata);
  private:
    redisContext* m_redisContext;
    std::string m_ipAddress;
    unsigned short m_port;
};