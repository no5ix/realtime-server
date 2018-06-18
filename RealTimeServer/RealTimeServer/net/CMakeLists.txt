set (PROJ_NAME udp_net)

IF(UNIX)

    set(udp_net_SRCS
      UdpAcceptor.cpp
      UdpClient.cpp
      UdpConnection.cpp
      UdpConnector.cpp
      UdpServer.cpp
      UdpSocketsOps.cpp
    )

    add_library(udp_net ${udp_net_SRCS})
    target_link_libraries(udp_net tcp_net)


    add_subdirectory(test/chat)

elseif(WIN32) # Check if we are on Windows

  create_win_proj(${PROJ_NAME})
    
else()
  	message(SEND_ERROR "You are on an unsupported platform! (Not Win32 or Unix)")
ENDIF()