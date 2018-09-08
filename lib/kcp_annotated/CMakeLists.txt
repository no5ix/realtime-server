# CMake 最低版本号要求
cmake_minimum_required (VERSION 2.8)

set (LIB_KCP kcp)

IF(UNIX)

    set(kcp_SRCS
      ikcp.c
    )

    add_library(${LIB_KCP} ${kcp_SRCS})
    target_link_libraries(${LIB_KCP})

    # support C++11
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

    add_executable(ServerTestKcpSession TestKcpSessionServer.cpp)
    target_link_libraries(ServerTestKcpSession ${LIB_KCP})


    add_executable(ClientTestKcpSession TestKcpSessionClient.cpp)
    target_link_libraries(ClientTestKcpSession ${LIB_KCP})

    message(STATUS  "TestKcpSession build finished")
    
else()
  	message(SEND_ERROR "You are on an unsupported platform! (Not Unix)")
ENDIF()