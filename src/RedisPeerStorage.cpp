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
        static_cast<redisReply*>(redisCommand(m_redisContext, "HSET peer:%s peerIp %s peerPort %s",
                                              peerInfo.peerUuid.c_str(), peerInfo.peerIp.c_str(), peerInfo.peerPort.c_str()));
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
        m_redisContext, "HSET file:%s:metadata fileName %s fileSize %lld fileDescription %s totalChunks %lld",
        metaData.fileNameUuid.c_str(), metaData.fileName.c_str(), metaData.fileSize, metaData.fileDescription.c_str(),
        metaData.totalChunks));
    freeReplyObject(reply);
    for (auto chunkId = 0; chunkId < metaData.totalChunks; chunkId++) {
        const auto& chunkHash = metaData.chunkHashes[chunkId];
        reply = static_cast<redisReply*>(redisCommand(m_redisContext, "HSET file:%s:chunks:%lld hash %s",
                                                      metaData.fileNameUuid.c_str(), chunkId, chunkHash.c_str()));
        freeReplyObject(reply);
        reply =
            static_cast<redisReply*>(redisCommand(m_redisContext, "SADD file:%s:chunks:%lld:peers %s",
                                                  metaData.fileNameUuid.c_str(), chunkId, metaData.peerUuid.c_str()));
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

bool RedisPeerStorage::updateChunkPeerList(const ChunkAdvertisement& chunkAdvert) {
    redisReply* reply = static_cast<redisReply*>(redisCommand(m_redisContext, "SADD file:%s:chunks:%lld:peers %s",
                                                              chunkAdvert.fileUuid.c_str(), chunkAdvert.chunkId,
                                                              chunkAdvert.peerUuid.c_str()));
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
    std::cout << "[DEBUG] Updated chunk peers." << std::endl;
    return true;
}

bool RedisPeerStorage::deleteInactivePeerFromChunkList(const ChunkAdvertisement& chunkAdvert) {
    redisReply* reply = static_cast<redisReply*>(redisCommand(m_redisContext, "SREM file:%s:chunks:%lld:peers %s",
                                                                chunkAdvert.fileUuid.c_str(), chunkAdvert.chunkId,
                                                                chunkAdvert.peerUuid.c_str()));
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
    std::cout << "[DEBUG] Deleted inactive peer " << chunkAdvert.peerUuid.c_str() 
              << "from file:" << chunkAdvert.fileUuid << ":chunks:" << chunkAdvert.chunkId
              << ":peers" << std::endl;
    return true;
}

std::vector<FileMetadata> RedisPeerStorage::retrieveAllFileDetails() {
    std::vector<FileMetadata> fileDetails;
    redisReply* reply = static_cast<redisReply*>(redisCommand(m_redisContext, "KEYS file:*:metadata"));
    if (reply == nullptr) {
        std::cerr << "Error: " << m_redisContext->errstr << std::endl;
        return fileDetails;
    }
    for (size_t i = 0; i < reply->elements; ++i) {
        std::string key = reply->element[i]->str;
        auto start = key.find(":") + 1;
        auto end = key.rfind(":metadata");
        FileMetadata metaData;
        if (start != std::string::npos && end != std::string::npos) {
            metaData.fileNameUuid = key.substr(start, end - start);
            redisReply* metaReply = static_cast<redisReply*>(
                redisCommand(m_redisContext, "HMGET %s fileName fileSize fileDescription", key.c_str()));
            if (metaReply && metaReply->elements == 3) {
                metaData.fileName = metaReply->element[0]->str;
                metaData.fileSize = std::stoull(metaReply->element[1]->str);
                metaData.fileDescription = metaReply->element[2]->str;
                fileDetails.push_back(std::move(metaData));
            }
            freeReplyObject(metaReply);
        }
    }
    return fileDetails;
}

// TODO: Pipeling commands to improve performance.
std::optional<FileMetadata> RedisPeerStorage::retrieveFileDetails(const std::string& uuid) {
    FileMetadata fileMetaData;
    redisReply* reply =
        static_cast<redisReply*>(redisCommand(m_redisContext, "HGET file:%s:metadata totalChunks", uuid.c_str()));

    if (!reply) {
        return std::nullopt;
    }
    fileMetaData.totalChunks = std::stoull(reply->str);
    freeReplyObject(reply);
    for (auto chunkIndex = 0; chunkIndex < fileMetaData.totalChunks; chunkIndex++) {
        ChunkInfo chunk;
        reply = static_cast<redisReply*>(
            redisCommand(m_redisContext, "HGET file:%s:chunks:%lld hash", uuid.c_str(), chunkIndex));
        if (!reply || reply->type != REDIS_REPLY_STRING) {
            std::cout << "Failed to retrieve hash" << std::endl;
            freeReplyObject(reply);
            return std::nullopt;
        }
        chunk.hash = reply->str;
        freeReplyObject(reply);
        reply = static_cast<redisReply*>(
            redisCommand(m_redisContext, "SMEMBERS file:%s:chunks:%lld:peers", uuid.c_str(), chunkIndex));
        if (!reply || reply->type != REDIS_REPLY_ARRAY) {
            std::cout << "Failed to fetch peers for chunk" << std::endl;
            freeReplyObject(reply);
            return std::nullopt;
        }
        for (auto peerIndex = 0; peerIndex < reply->elements; peerIndex++) {
            PeerInfo peerInfo;
            std::string peerUuid = reply->element[peerIndex]->str;
            redisReply* peerReply = static_cast<redisReply*>(
                redisCommand(m_redisContext, "HMGET peer:%s peerIp peerPort", peerUuid.c_str()));
            if (!peerReply || peerReply->type != REDIS_REPLY_ARRAY || peerReply->elements != 2) {
                std::cout << "Failed to get peer details from UUID" << std::endl;
                if (peerReply)
                    freeReplyObject(peerReply);
                continue;
            }
            if (peerReply->element[0]->type == REDIS_REPLY_STRING) {
                peerInfo.peerIp = peerReply->element[0]->str;
            }
            if (peerReply->element[1]->type == REDIS_REPLY_STRING) {
                peerInfo.peerPort = peerReply->element[1]->str;
            }
            peerInfo.peerUuid = std::move(peerUuid);
            freeReplyObject(peerReply);
            chunk.peers.push_back(std::move(peerInfo));
        }
        fileMetaData.chunkDetails.push_back(std::move(chunk));
    }
    return fileMetaData;
}