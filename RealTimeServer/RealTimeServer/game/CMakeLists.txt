set (PROJ_NAME rts)

IF(UNIX)

    file(GLOB_RECURSE all_files "./*.*")

    # 指定生成目标
    add_executable(${PROJ_NAME} ${all_files})
    # 添加链接库
    target_link_libraries(${PROJ_NAME} udp_net)  

elseif(WIN32) # Check if we are on Windows

  create_win_proj(${PROJ_NAME})

else()
  	message(SEND_ERROR "You are on an unsupported platform! (Not Win32 or Unix)")
ENDIF()