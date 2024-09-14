#include "P2PServer.hpp"

int main(int argc, char *argv[])
{
    std::string redisIp   = "127.0.0.1";
    int         redisPort = 6379;
    if (argc > 1)
    {
        redisIp = argv[1];
    }
    if (argc > 2)
    {
        redisPort = std::atoi(argv[2]);
    }
    P2PServer server;
    if (!server.connectToDatabase(redisIp, redisPort))
    {
        return 1;
    }
    server.startListening();
    return 0;
}