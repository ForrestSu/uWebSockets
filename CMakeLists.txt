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

list(APPEND add_cppflags -std=c++17 -g -O2) 
list(APPEND add_cppflags -Wall -Wextra)
list(APPEND add_cppflags -Wno-unused-parameter)
list(APPEND extra_so_flags -shared -fPIC)

list(APPEND add_libraries pthread crypto ssl z uv)
list(APPEND add_includes ./src)

file(GLOB project_headers
    ./src/Libuv.h
    ./src/Backend.h
    ./src/Extensions.h
    ./src/Group.h
    ./src/HTTPSocket.h
    ./src/Hub.h
    ./src/Networking.h
    ./src/Node.h
    ./src/Room.h
    ./src/Socket.h
    ./src/uWS.h
    ./src/WebSocket.h
    ./src/WebSocketProtocol.h)


set(project_sources
    ./src/Extensions.cpp 
    ./src/Group.cpp 
    ./src/Networking.cpp 
    ./src/Hub.cpp 
    ./src/Node.cpp 
    ./src/WebSocket.cpp 
    ./src/HTTPSocket.cpp
    ./src/Socket.cpp)

####### so ######
add_library(uWS_uv SHARED ${project_sources})
target_compile_options(uWS_uv PRIVATE ${add_cppflags} ${extra_so_flags})
target_include_directories(uWS_uv PRIVATE ${add_includes})
target_link_libraries(uWS_uv ${add_libraries})


###### sample ######
add_executable(http_svr ./tests/test_http.cpp)
target_compile_options(http_svr PRIVATE ${add_cppflags})
target_include_directories(http_svr PRIVATE ${add_includes})
target_link_libraries(http_svr ${add_libraries} uWS_uv)

if(UNIX)
  INSTALL(TARGETS uWS_uv LIBRARY DESTINATION /usr/lib64)
  INSTALL(FILES ${project_headers} DESTINATION /usr/include/uWebSockets)
endif()
