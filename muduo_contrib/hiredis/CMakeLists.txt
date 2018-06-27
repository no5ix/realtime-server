
set (MUDUO_LIB_NAME muduo)


add_executable(mrediscli Hiredis.cc mrediscli.cc)
target_link_libraries(mrediscli ${MUDUO_LIB_NAME} hiredis)
