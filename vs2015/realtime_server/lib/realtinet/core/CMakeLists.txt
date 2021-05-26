set (PROJ_NAME realtinet)
set (MUDUO_LIB_NAME muduo)
set (KCPSESS_LIB_NAME kcpsess)

IF(UNIX)

    set(realtinet_SRCS
      UdpAcceptor.cpp
      UdpClient.cpp
      UdpConnection.cpp
      UdpConnector.cpp
      UdpServer.cpp
      UdpSocketsOps.cpp
    )

    add_library(${PROJ_NAME} ${realtinet_SRCS})
    target_link_libraries(${PROJ_NAME} ${MUDUO_LIB_NAME} ${KCPSESS_LIB_NAME})

# elseif(WIN32) # Check if we are on Windows

#   create_win_proj(${PROJ_NAME})
    
else()
  	message(SEND_ERROR "You are on an unsupported platform! (Not Win32 or Unix)")
ENDIF()