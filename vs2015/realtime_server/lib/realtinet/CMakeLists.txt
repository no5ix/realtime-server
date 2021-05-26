
cmake_minimum_required (VERSION 2.8)

# CMake disable -std=c++11 flag for C files
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y")

project (realtinet)

set_property(GLOBAL PROPERTY USE_FOLDERS On)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)


if(UNIX)

    if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
        # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic")
        # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")  # Inhibit all warning messages.
        set(CXX_FLAGS
            -g
            # -DVALGRIND
            # -DMUDUO_STD_STRING
            -DCHECK_PTHREAD_RETURN_VALUE
            -D_FILE_OFFSET_BITS=64
            -Wall
            -Wextra
            -Werror
            # -Wconversion
            -Wno-unused-parameter
            # -Wold-style-cast
            -Wno-multichar
            -Wno-unused-function
            -Wno-reorder
            -Woverloaded-virtual
            -Wpointer-arith
            # -Wshadow
            -Wwrite-strings
            -march=native
            # -MMD
            -std=c++0x
            -rdynamic
        )
        string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")
    endif()

  	# For gdb
    set(CMAKE_BUILD_TYPE "Debug")
    set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
    set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")


    include_directories(${PROJECT_SOURCE_DIR}/core)
    include_directories(${PROJECT_SOURCE_DIR}/third)

    add_subdirectory(core)
    add_subdirectory(third/muduo)
    add_subdirectory(third/kcpsess)

else()
  	message(SEND_ERROR "You are on an unsupported platform! (Not Unix)")
ENDIF()