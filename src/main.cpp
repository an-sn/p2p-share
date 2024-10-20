#include "P2PServer.hpp"
#include <csignal>
#include <iostream>

P2PServer* g_p2pServer = nullptr;

void handleSignal(int signum) {
    if (g_p2pServer) {
        g_p2pServer->stopServer();
    }
}

int main(int argc, char* argv[]) {
    std::string redisIp = "127.0.0.1";
    int redisPort = 6379;
    int threadCount = 1;
    if (argc > 1) {
        redisIp = argv[1];
    }
    if (argc > 2) {
        redisPort = std::atoi(argv[2]);
    }
    if (argc > 3) {
        threadCount = std::atoi(argv[3]);
    }
    P2PServer server;
    g_p2pServer = &server;
    std::signal(SIGTERM, handleSignal);
    std::signal(SIGINT, handleSignal);
    if (!server.connectToDatabase(redisIp, redisPort)) {
        return 1;
    }
    server.startListening(threadCount);
    std::cout << "P2P Server exiting!" << std::endl;
    return 0;
}