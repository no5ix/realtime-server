

# support C++14
add_definitions(-std=c++1y)

set (PROJ_NAME rs_example_for_ue4_demo)
set (PROJ_NAME_PLUS rs_example_for_ue4_demo_plus)

set (LIB_REALTIME_SRV realtime_srv)
set (LIB_MUDUO_HIREDIS muduo_hiredis)
set (LIB_LUA_BIND lua_bind)

IF(UNIX)

    set(SRCS
        Character.cpp
        ExampleSrvForUe4Demo.cpp
    )
    add_executable(${PROJ_NAME} ${SRCS})
    target_link_libraries(${PROJ_NAME} ${LIB_REALTIME_SRV})
    message(STATUS  "${PROJ_NAME} build finished")
    

    if (CMAKE_COMPILER_IS_GNUCC)
        execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
        message(STATUS "current gcc Version is ${GCC_VERSION}")

        # (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 6.0) is not working.
        if (GCC_VERSION VERSION_GREATER 6.0 OR GCC_VERSION VERSION_EQUAL 6.0)

            if(HIREDIS_INCLUDE_DIR AND HIREDIS_LIBRARY)
                set(SRCS
                    Character.cpp
                    ExampleSrvForUe4DemoPlus.cpp
                    CharacterLuaBind.cpp
                    ExampleRedisCli.cpp
                )
                add_executable(${PROJ_NAME_PLUS} ${SRCS})
                target_link_libraries(${PROJ_NAME_PLUS} ${LIB_REALTIME_SRV} ${LIB_MUDUO_HIREDIS} ${LIB_LUA_BIND})
                message(STATUS  "${PROJ_NAME_PLUS} build finished.")
            endif()

        else()

            message(WARNING  "${PROJ_NAME} build succeeded, but ${PROJ_NAME_PLUS} build failed, ${PROJ_NAME_PLUS} Requires :
                    - Redis
                    - Hiredis 
                    - gcc Version >= 6.0 for compiling Sol2
                so just do `sh ./run_rs.sh` to play ${PROJ_NAME} for now ."
            )
                
        endif()
    endif()

# elseif(WIN32) # Check if we are on Windows

#   create_win_proj(${PROJ_NAME})
  

else()
  	message(SEND_ERROR "You are on an unsupported platform! (Not Win32 or Unix)")
ENDIF()
