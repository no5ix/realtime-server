set (PROJ_NAME lua_bind)

IF(UNIX)

    file(GLOB_RECURSE SRCS "./*.*")

    list(REMOVE_ITEM SRCS ${CMAKE_CURRENT_SOURCE_DIR}/lua53/lua.c)
    list(REMOVE_ITEM SRCS ${CMAKE_CURRENT_SOURCE_DIR}/lua53/luac.c)


    add_library(${PROJ_NAME} ${SRCS})
    target_link_libraries(${PROJ_NAME})

# elseif(WIN32) # Check if we are on Windows

#   create_win_proj(${PROJ_NAME})
    
else()
  	message(SEND_ERROR "You are on an unsupported platform! (Not Win32 or Unix)")
ENDIF()