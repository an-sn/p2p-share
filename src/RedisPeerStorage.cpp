#include "RedisPeerStorage.hpp"
#include "FileMetaData.hpp"

#include <iostream>

RedisPeerStorage::RedisPeerStorage() {
}

RedisPeerStorage::~RedisPeerStorage() {
    if (m_redisContext) {
        redisFree(m_redisContext);
    }
}

bool RedisPeerStorage::connect(std::string ipAddress, unsigned short port) {
    m_redisContext = redisConnect(ipAddress.c_str(), port);
    if (m_redisContext == nullptr) {
        return false;
    }
    if (m_redisContext->err) {
        std::string errorMessage = "Failed to connect to Redis: ";
        errorMessage += m_redisContext->errstr;
        redisFree(m_redisContext);
        m_redisContext = nullptr;
        std::cout << errorMessage << std::endl;
        return false;
    }
    std::cout << "Successfully connected to Redis DB!" << std::endl;
    return true;
}

bool RedisPeerStorage::storePeerInfo(const std::string& uuid, const std::string& peerIp, const std::string& peerPort) {
    if (!m_redisContext) {
        std::cerr << "Failed to save peer info. Redis connection is not established." << std::endl;
        return false;
    }
    std::string command = "HSET " + uuid + " peer_ip " + peerIp + " peer_port " + peerPort;
    redisReply* reply = static_cast<redisReply*>(redisCommand(m_redisContext, command.c_str()));
    if (reply == nullptr) {
        std::cerr << "Error executing command: " << m_redisContext->errstr << std::endl;
        return false;
    }
    auto replyType = reply->type;
    freeReplyObject(reply);
    if (replyType != REDIS_REPLY_INTEGER) {
        std::cerr << "Unexpected reply type: " << replyType << std::endl;
        return false;
    }
    return true;
}

bool RedisPeerStorage::storeFileMetadata(FileMetadata &fileMetadata)
{
    return true;
}