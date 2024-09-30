#include "RedisPeerStorage.hpp"
#include "FileMetaData.hpp"
#include "PeerInfo.hpp"

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

bool RedisPeerStorage::storePeerInfo(const PeerInfo& peerInfo) {
    if (!m_redisContext) {
        std::cerr << "Failed to save peer info. Redis connection is not established." << std::endl;
        return false;
    }
    redisReply* reply =
        static_cast<redisReply*>(redisCommand(m_redisContext, "HSET peer:%s peerIp %s peerPort %lld",
                                              peerInfo.peerUuid.c_str(), peerInfo.peerIp.c_str(), peerInfo.peerPort));
    if (!reply) {
        std::cerr << "Error executing command: " << m_redisContext->errstr << std::endl;
        return false;
    }
    auto replyType = reply->type;
    freeReplyObject(reply);
    if (replyType != REDIS_REPLY_INTEGER) {
        std::cerr << "Unexpected reply type: " << replyType << std::endl;
        return false;
    }
    std::cout << "[DEBUG] PEER DATA SAVED." << std::endl;
    return true;
}

bool RedisPeerStorage::storeFileMetadata(const FileMetadata& metaData) {
    if (!m_redisContext) {
        std::cerr << "Failed to store file metadata. Redis connection is not established." << std::endl;
        return false;
    }
    redisReply* reply = static_cast<redisReply*>(redisCommand(m_redisContext, "MULTI"));
    freeReplyObject(reply);
    reply = static_cast<redisReply*>(redisCommand(
        m_redisContext, "HSET file:%s:metadata fileName %s fileSize %lld totalChunks %lld",
        metaData.fileNameUuid.c_str(), metaData.fileName.c_str(), metaData.fileSize, metaData.totalChunks));
    freeReplyObject(reply);
    for (auto chunkId = 0; chunkId < metaData.totalChunks; chunkId++) {
        const auto& chunkHash = metaData.chunkHashes[chunkId];
        reply = static_cast<redisReply*>(redisCommand(m_redisContext, "HSET file:%s:chunks:%lld hash %s",
                                                      metaData.fileNameUuid.c_str(), chunkId, chunkHash.c_str()));
        freeReplyObject(reply);
    }
    reply = static_cast<redisReply*>(redisCommand(m_redisContext, "EXEC"));
    if (!reply) {
        std::cerr << "Failed to complete file transaction!" << std::endl;
        return false;
    }
    if (reply->type != REDIS_REPLY_ARRAY) {
        std::cerr << "Transaction execution failed: Unexpected reply type: " << reply->type << std::endl;
        freeReplyObject(reply);
        return false;
    }
    for (size_t i = 0; i < reply->elements; ++i) {
        if (reply->element[i]->type != REDIS_REPLY_INTEGER) {
            std::cerr << "Failed to save metadata or chunk data!" << std::endl;
            freeReplyObject(reply);
            return false;
        }
    }
    freeReplyObject(reply);
    std::cout << "[DEBUG] Metadata and chunk data saved successfully." << std::endl;
    return true;
}