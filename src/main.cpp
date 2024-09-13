#include <iostream>
#include "HttpServer.hpp"
// #include "InMemoryPeerStorage.hpp"
#include "RedisPeerStorage.hpp"

int main() {
    try {
        // Create I/I/O context
        net::io_context ioc;

        // Create and open acceptor
        tcp::acceptor acceptor(ioc, tcp::endpoint(tcp::v4(), 8080));
        acceptor.set_option(net::socket_base::reuse_address(true));
        RedisPeerStorage storage("192.168.0.11", 6379);
        storage.connectToRedisDb();
        // Create and start the HTTP server
        HttpServer server(ioc, acceptor);
        server.start();

        // Run the I/O context
        ioc.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 0;
}