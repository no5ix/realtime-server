set (LIB_REALTIME_SRV realtime_srv)
set (PROJ_NAME rs_example_for_ue4_demo)

IF(UNIX)

    file(GLOB_RECURSE all_files "./*.*")

    add_executable(${PROJ_NAME} ${all_files})
    target_link_libraries(${PROJ_NAME} ${LIB_REALTIME_SRV})

# elseif(WIN32) # Check if we are on Windows

#   create_win_proj(${PROJ_NAME})
  

else()
  	message(SEND_ERROR "You are on an unsupported platform! (Not Win32 or Unix)")
ENDIF()
