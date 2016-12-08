cmake_minimum_required(VERSION 2.8)

cmaki_find_package(teelogging)
cmaki2_executable(server test_server.cpp DEPENDS channel_service PTHREADS)
cmaki2_executable(client test_client.cpp DEPENDS channel_service PTHREADS)
