project("hzlisp")

cmake_minimum_required(VERSION 3.15)

add_executable(hz main.c mpc.c hzval.c builtin.c builtin/alloc.c builtin/hashmap.c)
target_link_libraries(hz -ledit)