set_property(GLOBAL PROPERTY USE_FOLDERS On)

# CMake 最低版本号要求
cmake_minimum_required (VERSION 2.8)

# support C++11
add_definitions(-std=c++11)



# 项目信息
set (PROJ_NAME realtime_srv)
project (${PROJ_NAME})

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

# 查找当前目录下的所有源文件
# 并将名称保存到 DIR_SRCS 变量
# aux_source_directory(./${PROJ_NAME} DIR_SRCS)


# 查找当前目录以及子目录下的所有头文件
# 并将名称保存到 CURRENT_HEADERS 变量
# file(GLOB_RECURSE CURRENT_HEADERS  *.h *.hpp)


# 此命令可以用来收集源文件 CURRENT_HEADERS 作为变量保存收集的结果。 
# 后面为文件过滤器，其中 PROJ_NAME 为起始搜索的文件夹，即在 realtime_srv 目录下，
# 开始收集，而且会遍历子目录
# file(
#     GLOB_RECURSE CURRENT_HEADERS 
#     LIST_DIRECTORIES false
#     "${PROJ_NAME}/*.h*"
# )

# 生成一个名为Include的VS筛选器
# source_group("Include" FILES ${CURRENT_HEADERS}) 


# IF(WIN32) # Check if we are on Windows

#     macro(create_win_proj win_proj_name)
#         if(MSVC) # Check if we are using the Visual Studio compiler

#             file(GLOB_RECURSE project_headers *.h)
#             file(GLOB_RECURSE project_cpps *.c*)
#             set(all_files ${project_headers} ${project_cpps})

#             # 获取当前目录
#             set(current_dir ${CMAKE_CURRENT_SOURCE_DIR})
#             foreach(src_file ${all_files})
#                 # 求出相对路径
#                 string(REPLACE ${current_dir}/ "" rel_path_name ${src_file})
#                 # 删除相对路径中的文件名部分
#                 string(REGEX REPLACE "(.*)/.*" \\1 rel_path ${rel_path_name})
#                 # 比较是否是当前路径下的文件
#                 string(COMPARE EQUAL ${rel_path_name} ${rel_path} is_same_path)
#                 # 替换成Windows平台的路径分隔符
#                 string(REPLACE "/" "\\" rel_path ${rel_path})
#                 if(is_same_path)
#                     set(rel_path "\\")
#                 endif(is_same_path)

#                 # CMake 命令
#                 source_group(${rel_path} FILES ${src_file})
#             endforeach(src_file)
    
#             # 指定生成目标
#             add_executable(${win_proj_name} ${all_files})

#             # set_target_properties(${win_proj_name} PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS") # works for all build modes
#             set_target_properties(${win_proj_name} PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE") # works for all build modes
#             set_target_properties(${win_proj_name} PROPERTIES FOLDER "realtime_srv")
#             target_link_libraries(${win_proj_name} wsock32 ws2_32)
#             set_target_properties(${win_proj_name}
#                 PROPERTIES
#                 COMPILE_FLAGS /wd"4819"
#             )

#         elseif(CMAKE_COMPILER_IS_GNUCXX)
#             # SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mwindows") # Not tested
#         else()
#             message(SEND_ERROR "You are using an unsupported Windows compiler! (Not MSVC or GCC)")
#         endif(MSVC)
#     endmacro(create_win_proj)

#     include_directories(${PROJECT_SOURCE_DIR})

#     add_subdirectory(muduo)
#     add_subdirectory(muduo_udp_support)
#     add_subdirectory(realtime_srv)
    
#     add_subdirectory(example)


# elseif(UNIX)

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
            -Wno-reorder
            -Woverloaded-virtual
            -Wpointer-arith
            -Wshadow
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

    # find hiredis
    find_path(HIREDIS_INCLUDE_DIR hiredis/hiredis.h)
    find_library(HIREDIS_LIBRARY NAMES hiredis)

    if(HIREDIS_INCLUDE_DIR AND HIREDIS_LIBRARY)
        message(STATUS "found hiredis")
    endif()


    include_directories(${PROJECT_SOURCE_DIR})

    add_subdirectory(muduo)
    add_subdirectory(muduo_udp_support)
    add_subdirectory(muduo_contrib)

    add_subdirectory(realtime_srv)
    add_subdirectory(example)

else()
  	message(SEND_ERROR "You are on an unsupported platform! (Not Win32 or Unix)")
ENDIF()