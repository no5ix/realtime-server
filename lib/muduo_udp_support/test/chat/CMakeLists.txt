set (LIB_NAME muduo_udp_support)

IF(UNIX)

    add_executable(udp_chat_cli udpChatCli.cpp)
    target_link_libraries(udp_chat_cli ${PROJ_NAME})

    add_executable(udp_chat_srv udpChatSrv.cpp)
    target_link_libraries(udp_chat_srv ${PROJ_NAME})

    add_executable(udp_chat_srv_highperformance udpChatSrv_highperformance.cpp)
    target_link_libraries(udp_chat_srv_highperformance ${PROJ_NAME})
    
else()
  	message(SEND_ERROR "You are on an unsupported platform! (Not Win32 or Unix)")
ENDIF()