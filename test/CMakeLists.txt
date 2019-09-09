# @Reference: https://github.com/uNetworking/uWebSockets
# @Author: sunquan
# @Date: 2019-08-30 18:08:41

# This CMakeLists.txt is for test!!!!
##


cmake_minimum_required(VERSION 2.8)
project(http_svr)

list(APPEND add_cppflags -std=c++17 -g -O2) 
list(APPEND add_cppflags -Wall -Wextra)
list(APPEND add_cppflags -Wno-unused-parameter)

list(APPEND add_libraries z uv)
list(APPEND add_includes /usr/include/uWebSockets)


set(project_sources HttpServer.cpp)

###### sample ######
add_executable(http_svr ${project_sources})

target_compile_definitions(http_svr PRIVATE ${add_defines})
target_compile_options(http_svr PRIVATE ${add_cppflags})
target_include_directories(http_svr PRIVATE ${add_includes})
target_link_libraries(http_svr ${add_libraries} uWS_uv)

