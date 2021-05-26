include_directories(../include)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")

if (BUILD_SHARED_LIBS)
  add_library(inih SHARED ini.c)
  add_library(inihcpp SHARED ini.c INIReader.cpp)
else(BUILD_SHARED_LIBS)
  if (WIN32 AND STATIC_LINK_MSVCRT)
    # embed the visual studio runtime dll (msvcrt*.dll) into this library
    string(REPLACE "/MD" "" CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MT")
    string(REPLACE "/MDd" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MTd")
  endif (WIN32 AND STATIC_LINK_MSVCRT)

  add_library(inih STATIC ini.c)
  add_library(inihcpp STATIC ini.c INIReader.cpp)
endif(BUILD_SHARED_LIBS)

install(TARGETS inih inihcpp DESTINATION lib)

