
# support C++14
add_definitions(-std=c++1y)


set (PROJ_NAME realtime_srv)
set (LIB_MUDUO_UDP_SUPPORT realtinet)
set (LIB_INIH inihcpp) # config file parser

IF(UNIX)

    file(GLOB_RECURSE all_files "./*.*")

    add_library(${PROJ_NAME} ${all_files})
    target_link_libraries(${PROJ_NAME} ${LIB_MUDUO_UDP_SUPPORT} ${LIB_INIH})

# elseif(WIN32) # Check if we are on Windows

#   create_win_proj(${PROJ_NAME})
  

else()
  	message(SEND_ERROR "You are on an unsupported platform! (Not Win32 or Unix)")
ENDIF()