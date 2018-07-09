set (PROJ_NAME rs_example_for_ue4_demo)
set (PROJ_NAME_PLUS rs_example_for_ue4_demo_plus)

set (LIB_REALTIME_SRV realtime_srv)
set (LIB_MUDUO_HIREDIS muduo_hiredis)
set (LIB_LUA lua${LUA_VERSION})


IF(UNIX)

    if(HIREDIS_INCLUDE_DIR AND HIREDIS_LIBRARY AND LUA_INCLUDE_DIR AND LUA_LIBRARY)

        set(example_for_ue4_demo_SRCS
            Character.cpp
            ExampleSrvForUe4DemoPlus.cpp
            CharacterLuaBind.cpp
            ExampleRedisCli.cpp
        )
        add_executable(${PROJ_NAME_PLUS} ${example_for_ue4_demo_SRCS})
        target_link_libraries(${PROJ_NAME_PLUS} ${LIB_REALTIME_SRV} ${LIB_MUDUO_HIREDIS} ${LIB_LUA})
    
    endif()

    set(example_for_ue4_demo_SRCS
        Character.cpp
        ExampleSrvForUe4Demo.cpp
    )
    add_executable(${PROJ_NAME} ${example_for_ue4_demo_SRCS})
    target_link_libraries(${PROJ_NAME} ${LIB_REALTIME_SRV})


# elseif(WIN32) # Check if we are on Windows

#   create_win_proj(${PROJ_NAME})
  

else()
  	message(SEND_ERROR "You are on an unsupported platform! (Not Win32 or Unix)")
ENDIF()
