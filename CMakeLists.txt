cmake_minimum_required(VERSION 3.10)
project(p2p_file_sharing)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost 1.83.0 COMPONENTS system REQUIRED)

include_directories(include/)

add_executable(p2p_server
    src/main.cpp
    src/InMemoryPeerStorage.cpp
    src/HttpServer.cpp
)