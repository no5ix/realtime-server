set (PROJ_NAME tcp_net)

IF(UNIX)

  # aux_source_directory(. tcp_net_SRCS)
  file(GLOB_RECURSE tcp_net_SRCS "./*.*")

  add_library(tcp_net ${tcp_net_SRCS})
  target_link_libraries(tcp_net pthread rt)

elseif(WIN32) # Check if we are on Windows

  create_win_proj(${PROJ_NAME})
    
else()
  	message(SEND_ERROR "You are on an unsupported platform! (Not Win32 or Unix)")
ENDIF()