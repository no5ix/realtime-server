
set (MUDUO_LIB_NAME muduo)
set (PROJ_NAME muduo_hiredis)

IF(UNIX)

    file(GLOB_RECURSE muduo_hiredis_SRCS "./*.*")
    add_library(${PROJ_NAME} ${muduo_hiredis_SRCS})
    target_link_libraries(${PROJ_NAME} ${MUDUO_LIB_NAME} hiredis)

# elseif(WIN32) # Check if we are on Windows

#   create_win_proj(${PROJ_NAME})
    
else()
  	message(SEND_ERROR "You are on an unsupported platform! (Not Win32 or Unix)")
ENDIF()

