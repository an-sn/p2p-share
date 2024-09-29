#pragma once

#include <hiredis/hiredis.h>

#include <string>

struct PeerInfo;

class RedisPeerStorage {
  public:
    RedisPeerStorage();
    ~RedisPeerStorage();

    bool connect(std::string ipAddress, unsigned short port);

  private:
    redisContext* m_redisContext;
    std::string m_ipAddress;
    unsigned short m_port;
};