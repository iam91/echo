cmake_minimum_required(VERSION 3.7)
project(echo)

set(CMAKE_CXX_STANDARD 11)

set(ECHO_SOURCE_FILES
        main.cpp)
set(CLIENT_SOURCE_FILES
        client.cpp)

add_executable(echo ${ECHO_SOURCE_FILES})
add_executable(client ${CLIENT_SOURCE_FILES})