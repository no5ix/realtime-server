set (PROJ_NAME muduo_udp_support)

IF(UNIX)

    set(muduo_udp_support_SRCS
      UdpAcceptor.cpp
      UdpClient.cpp
      UdpConnection.cpp
      UdpConnector.cpp
      UdpServer.cpp
      UdpSocketsOps.cpp
    )

    add_library(${PROJ_NAME} ${muduo_udp_support_SRCS})
    target_link_libraries(${PROJ_NAME} muduo)

    set (test_chat_path "${CMAKE_CURRENT_SOURCE_DIR}/test/chat")
    if (EXISTS ${test_chat_path})
  	  message(STATUS "test_chat_path is EXISTS.")
      add_subdirectory(test/chat)
    endif()

elseif(WIN32) # Check if we are on Windows

  create_win_proj(${PROJ_NAME})
    
else()
  	message(SEND_ERROR "You are on an unsupported platform! (Not Win32 or Unix)")
ENDIF()