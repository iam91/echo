cmake_minimum_required(VERSION 3.7)
project(echo)

find_library(HTTP_PARSER_LOC http_parser)
message("http_parser is at: ${HTTP_PARSER_LOC}")

set(CMAKE_CXX_STANDARD 11)
set(ECHO_SOURCE_FILES main.cpp socket.cpp socket.h server.cpp server.h error.cpp error.h parse.cpp parse.h)
set(CLIENT_SOURCE_FILES client.cpp)

add_executable(echo ${ECHO_SOURCE_FILES})
target_link_libraries(echo pthread)
target_link_libraries(echo ${HTTP_PARSER_LOC})
add_executable(client ${CLIENT_SOURCE_FILES})