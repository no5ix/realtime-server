# support C++11
add_definitions(-std=c++11)

# CMake 最低版本号要求
cmake_minimum_required (VERSION 2.8)

# 项目信息
set (PROJ_NAME ActionServer)
project (${PROJ_NAME})


#设置执行文件输出目录
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
#设置库输出路径
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# 查找当前目录下的所有源文件
# 并将名称保存到 DIR_SRCS 变量
aux_source_directory(./${PROJ_NAME} DIR_SRCS)

# 查找当前目录下的所有头文件
# 并将名称保存到 CURRENT_HEADERS 变量
file(GLOB_RECURSE CURRENT_HEADERS  *.h *.hpp)
source_group("Include" FILES ${CURRENT_HEADERS}) 

# 指定生成目标
add_executable(${PROJ_NAME} ${DIR_SRCS} ${CURRENT_HEADERS})

IF(WIN32) # Check if we are on Windows
  if(MSVC) # Check if we are using the Visual Studio compiler
    set_target_properties(${PROJ_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS") # works for all build modes
	  target_link_libraries(${PROJ_NAME} wsock32 ws2_32)
  elseif(CMAKE_COMPILER_IS_GNUCXX)
    # SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mwindows") # Not tested
  else()
    message(SEND_ERROR "You are using an unsupported Windows compiler! (Not MSVC or GCC)")
  endif()
elseif(UNIX)
  # Nothing special required
else()
  message(SEND_ERROR "You are on an unsupported platform! (Not Win32 or Unix)")
ENDIF()
