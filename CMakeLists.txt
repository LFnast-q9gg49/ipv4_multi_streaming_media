cmake_minimum_required(VERSION 3.22)
project(ipv4)

set(CMAKE_CXX_STANDARD 14)

add_executable(ipv4 ./src/client/main.cpp ./src/client/client.cpp ./src/client/client.h ./src/include/proto.h)