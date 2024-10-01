cmake_minimum_required(VERSION 3.10)
project(p2p_file_sharing)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost 1.83.0 COMPONENTS system json REQUIRED)

include_directories(include/)

add_compile_options(-Wunused-variable)

add_executable(p2p_server
    src/main.cpp
    src/HttpServer.cpp
    src/RedisPeerStorage.cpp
    src/P2PServer.cpp
    src/Utils.cpp
)

target_link_libraries(p2p_server
    Boost::system
    Boost::json
    hiredis)

target_include_directories(p2p_server PRIVATE ${Boost_INCLUDE_DIRS})