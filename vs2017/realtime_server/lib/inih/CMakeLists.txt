cmake_minimum_required(VERSION 2.6)
project(inih)

add_subdirectory(lib)
add_subdirectory(include)

if( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )
  set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-fPIC")
endif( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )

configure_file(inih.pc.in inih.pc @ONLY)
configure_file(inihcpp.pc.in inihcpp.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/inih.pc ${CMAKE_BINARY_DIR}/inihcpp.pc DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)
