project(channel_service CXX)
cmake_minimum_required(VERSION 3.2.2)

set(CMAKI_REPOSITORY "http://artifacts.myftp.biz:8080")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmaki/cmaki.cmake")
cmaki_setup()

cmaki_find_package(raknet)
cmaki_find_package(cryptopp)
cmaki_find_package(fast-event-system)
cmaki_find_package(cppunix)
cmaki2_library(channel_service base.cpp client.cpp server.cpp node.cpp)

add_subdirectory(tests)
