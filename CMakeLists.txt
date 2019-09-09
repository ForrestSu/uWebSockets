# @Reference: https://github.com/uNetworking/uWebSockets
# @Author: sunquan
# @Date: 2019-08-30 18:08:41

# This CMakeLists.txt is only for build uWS with libuv!!
#  Changlog:
#  (1) use -DUSE_LIBUV is default 
#  (2) "Asio.h" "Epoll.h" remove from header
#  (3) "Epoll.cpp"  remove from source code
##


cmake_minimum_required(VERSION 2.8)
project(uWS_uv)

## use c11 syntax
list(APPEND add_cppflags -std=c11 -g -O3) 
list(APPEND add_cppflags -Wall -Wextra)
list(APPEND add_cppflags -Wno-unused-parameter)
list(APPEND extra_so_flags -shared -fPIC)

list(APPEND add_includes ./uSockets/src)

## No SSL
list(APPEND add_defines LIBUS_NO_SSL)
#list(APPEND add_libraries ssl crypto)

## use libuv
list(APPEND add_defines LIBUS_USE_LIBUV)
list(APPEND add_libraries uv)


file(GLOB project_headers ./uSockets/src/libusockets.h)
file(GLOB project_cpp_headers ./src/*.h)
file(GLOB project_hpp_headers ./src/f2/function2.hpp)

file(GLOB project_sources
    ./uSockets/src/*.c 
    ./uSockets/src/eventing/libuv.c)


####### so ######
add_library(uWS_uv SHARED ${project_sources})

target_compile_definitions(uWS_uv PRIVATE ${add_defines})
target_compile_options(uWS_uv PRIVATE ${add_cppflags} ${extra_so_flags})
target_include_directories(uWS_uv PRIVATE ${add_includes})
target_link_libraries(uWS_uv ${add_libraries})
######## end so #########

if(UNIX)
  INSTALL(TARGETS uWS_uv LIBRARY DESTINATION /usr/lib64)
  INSTALL(FILES ${project_headers} ${project_cpp_headers} DESTINATION /usr/include/uWebSockets)
  INSTALL(FILES ${project_hpp_headers} DESTINATION /usr/include/uWebSockets/f2)
endif()
