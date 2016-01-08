cmake_minimum_required (VERSION 2.8)

add_definitions(-std=c++11)
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -Wall -g -ggdb ")
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -Wall")

project (server1)

aux_source_directory(. DIR_SRCS)

add_executable(server1 ${DIR_SRCS})

link_directories(/usr/local/lib)

target_link_libraries(server1 boost_system boost_thread)
