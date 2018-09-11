set (LIB_NAME muduo)

IF(UNIX)

  # aux_source_directory(. muduo_SRCS)
  file(GLOB_RECURSE muduo_SRCS "./*.*")

  add_library(${LIB_NAME} ${muduo_SRCS})
  target_link_libraries(${LIB_NAME} pthread rt)

# elseif(WIN32) # Check if we are on Windows

#   create_win_proj(${LIB_NAME})
    
else()
  	message(SEND_ERROR "You are on an unsupported platform! (Not Win32 or Unix)")
ENDIF()