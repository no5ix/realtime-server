set (LIB_KCP kcp)

IF(UNIX)

    set(kcp_SRCS
      ikcp.c
    )

    add_library(${LIB_KCP} ${kcp_SRCS})
    target_link_libraries(${LIB_KCP})

    add_executable(TestKcpSessionServer TestKcpSessionServer.cpp)
    target_link_libraries(TestKcpSessionServer ${LIB_KCP})


    add_executable(TestKcpSessionClient TestKcpSessionClient.cpp)
    target_link_libraries(TestKcpSessionClient ${LIB_KCP})

    message(STATUS  "TestKcpSession build finished")

# elseif(WIN32) # Check if we are on Windows

#   create_win_proj(${PROJ_NAME})
    
else()
  	message(SEND_ERROR "You are on an unsupported platform! (Not Win32 or Unix)")
ENDIF()