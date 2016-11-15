cmake_minimum_required(VERSION 2.8)

cmaki2_gtest(test_server test_server.cpp DEPENDS serializer PTHREADS)
cmaki2_gtest(test_client test_client.cpp DEPENDS serializer PTHREADS)

