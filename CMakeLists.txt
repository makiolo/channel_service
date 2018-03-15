project(channel_service CXX)
cmake_minimum_required(VERSION 3.0)

include("$ENV{CMAKI_PWD}/node_modules/npm-mas-mas/cmaki/cmaki.cmake")

cmaki_find_package(coroutine)
cmaki_find_package(raknet)
cmaki_find_package(boost-headers)
cmaki_find_package(boost-context)
cmaki_find_package(boost-coroutine2)
cmaki_find_package(boost-filesystem)
cmaki_find_package(boost-regex)
# cmaki_find_package(cryptopp)
cmaki_find_package(fast-event-system)
cmaki_find_package(asyncply)
cmaki_find_package(paho-mqttpp3)
cmaki_find_package(cppunix)
cmaki_library(channel_service base.cpp client.cpp server.cpp node.cpp)

add_subdirectory(tests)

