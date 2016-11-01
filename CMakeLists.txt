PROJECT(serialize CXX)
cmake_minimum_required(VERSION 3.4)

set(CMAKI_REPOSITORY "http://192.168.1.3:8080")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmaki/cmaki.cmake")
cmaki_setup()

cmaki_find_package(fast-event-system)
cmaki_find_package(raknet)
cmaki2_library(serializer network_base.cpp network_client.cpp network_server.cpp network_node.cpp)
cmaki2_executable(test_server test_server.cpp DEPENDS serializer PTHREADS)
cmaki2_executable(test_client test_client.cpp DEPENDS serializer PTHREADS)

