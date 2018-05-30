IF(UNIX)

    # for tcp
    add_executable(chat_client_tcp client.cc)
    target_link_libraries(chat_client_tcp udp_net)

    add_executable(chat_srv_highperformance_tcp server_threaded_highperformance.cc)
    target_link_libraries(chat_srv_highperformance_tcp udp_net)

    # for udp
    add_executable(udp_chat_cli udpChatCli.cpp)
    target_link_libraries(udp_chat_cli udp_net)

    add_executable(udp_chat_srv udpChatSrv.cpp)
    target_link_libraries(udp_chat_srv udp_net)

    add_executable(udp_chat_srv_highperformance udpChatSrv_highperformance.cpp)
    target_link_libraries(udp_chat_srv_highperformance udp_net)
    
else()
  	message(SEND_ERROR "You are on an unsupported platform! (Not Win32 or Unix)")
ENDIF()