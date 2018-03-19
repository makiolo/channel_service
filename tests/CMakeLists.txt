cmaki_executable(server test_server.cpp DEPENDS channel_service PTHREADS)
cmaki_executable(client test_client.cpp DEPENDS channel_service PTHREADS)
cmaki_executable(download test_download.cpp DEPENDS channel_service PTHREADS)

