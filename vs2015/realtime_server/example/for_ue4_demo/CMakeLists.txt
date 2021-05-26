

# support C++14
add_definitions(-std=c++1y)

set (EXAMPLE_NAME rs_example_for_ue4_demo)

set (LIB_REALTIME_SRV realtime_srv)

set (EXAMPLE_PLUS_NAME rs_example_for_ue4_demo_plus)
set (LIB_MUDUO_HIREDIS muduo_hiredis)
set (LIB_LUA_BIND lua_bind)

IF(UNIX)

    set(SRCS
        Character.cpp
        ExampleSrvForUe4Demo.cpp
    )
    add_executable(${EXAMPLE_NAME} ${SRCS})
    target_link_libraries(${EXAMPLE_NAME} ${LIB_REALTIME_SRV})
    message(STATUS  "${EXAMPLE_NAME} check finished")
    

    if (CMAKE_COMPILER_IS_GNUCC)
        execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
        message(STATUS "current gcc Version is ${GCC_VERSION}")

        # (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 5.4) is not working.
        if (GCC_VERSION VERSION_GREATER 5.4 OR GCC_VERSION VERSION_EQUAL 5.4)

            if(HIREDIS_INCLUDE_DIR AND HIREDIS_LIBRARY)
                set(SRCS
                    Character.cpp
                    ExampleSrvForUe4DemoPlus.cpp
                    CharacterLuaBind.cpp
                    ExampleRedisCli.cpp
                )
                add_executable(${EXAMPLE_PLUS_NAME} ${SRCS})
                target_link_libraries(${EXAMPLE_PLUS_NAME} ${LIB_REALTIME_SRV} ${LIB_MUDUO_HIREDIS} ${LIB_LUA_BIND})
                message(STATUS  "${EXAMPLE_PLUS_NAME} check finished.")
            endif()

        else()

            message(WARNING  "${EXAMPLE_NAME} check succeeded, but ${EXAMPLE_PLUS_NAME} check failed, ${EXAMPLE_PLUS_NAME} Requires :
                    - Redis
                    - Hiredis 
                    - gcc Version >= 5.4 for compiling Sol2
                so just do `sh ./run_rs.sh` to play ${EXAMPLE_NAME} for now ."
            )
                
        endif()
    endif()

# elseif(WIN32) # Check if we are on Windows

#   create_win_proj(${EXAMPLE_NAME})
  

else()
  	message(SEND_ERROR "You are on an unsupported platform! (Not Win32 or Unix)")
ENDIF()
