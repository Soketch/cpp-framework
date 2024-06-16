cmake_minimum_required(VERSION 2.8.12)
project(sylar)

include(cmake/utils.cmake)


set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined")

# include_directories(/apps/sylar/include)
#include_directories(/usr/include)
include_directories(.)
include_directories(/home/coding/tools)
link_directories(/usr/local/lib)

find_library(YAMLCPP yaml-cpp)
message("***", ${YAMLCPP})


set(LIB_SRC
    sylar/log.cc
    sylar/util.cc
    sylar/config.cc
    sylar/thread.cc
    sylar/mutex.cc
    sylar/fiber.cc
    sylar/scheduler.cc
    sylar/iomanager.cc
    sylar/timer.cc
    sylar/hook.cc
    sylar/fd_manager.cc
    sylar/address.cc
    sylar/socket.cc
    sylar/bytearray.cc
    sylar/stream.cc
    sylar/tcp_server.cc
    sylar/http/http.cc #http
    sylar/http/http11_parser.rl.cc
    sylar/http/httpclient_parser.rl.cc
    sylar/http/http_parser.cc
    sylar/http/http_session.cc
    sylar/http/http_connection.cc
    sylar/http/http_server.cc
    sylar/http/servlet.cc
    sylar/streams/socket_stream.cc #streams
    )

## ragel编译处理rl文件
ragelmaker(sylar/http/http11_parser.rl LIB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/sylar/http)
ragelmaker(sylar/http/httpclient_parser.rl LIB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/sylar/http)

ragelmaker(sylar/uri.rl LIB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/sylar)


# 生成共享库
add_library(sylar SHARED ${LIB_SRC})
force_redefine_file_macro_for_sources(sylar) 
#add_library(sylar_static STATIC ${LIB_SRC})
#set_target_properties(sylar_static PROPERTIES OUTPUT_NAME "sylar")


set(LIB_LIB
    sylar
    pthread
    ${YAMLCPP}
    )
    message("***", ${LIB_LIB})

add_executable(test tests/test.cc)
add_dependencies(test sylar)
force_redefine_file_macro_for_sources(test)   #__FILE__
target_link_libraries(test ${LIB_LIB})
 
add_executable(test_config tests/test_config.cc)
add_dependencies(test_config sylar)
force_redefine_file_macro_for_sources(test_config)  #__FILE__
target_link_libraries(test_config sylar ${LIB_LIB})

add_executable(test_thread tests/test_thread.cc)
add_dependencies(test_thread sylar)
force_redefine_file_macro_for_sources(test_thread)  #__FILE__
target_link_libraries(test_thread sylar ${LIB_LIB})
 
add_executable(test_util tests/test_util.cc)
add_dependencies(test_util sylar)
force_redefine_file_macro_for_sources(test_util)
target_link_libraries(test_util sylar ${LIB_LIB})

add_executable(test_fiber tests/test_fiber.cc)
add_dependencies(test_fiber sylar)
force_redefine_file_macro_for_sources(test_fiber)
target_link_libraries(test_fiber sylar ${LIB_LIB})

add_executable(test_scheduler tests/test_scheduler.cc)
add_dependencies(test_scheduler sylar)
force_redefine_file_macro_for_sources(test_scheduler)
target_link_libraries(test_scheduler sylar ${LIB_LIB})

add_executable(test_iomanager tests/test_iomanager.cc)
add_dependencies(test_iomanager sylar)
force_redefine_file_macro_for_sources(test_iomanager)
target_link_libraries(test_iomanager sylar ${LIB_LIB})

add_executable(test_hook tests/test_hook.cc)
add_dependencies(test_hook sylar)
force_redefine_file_macro_for_sources(test_hook)
target_link_libraries(test_hook sylar ${LIB_LIB})

add_executable(test_address tests/test_address.cc)
add_dependencies(test_address sylar)
force_redefine_file_macro_for_sources(test_address)
target_link_libraries(test_address sylar ${LIB_LIB})

add_executable(test_socket tests/test_socket.cc)
add_dependencies(test_socket sylar)
force_redefine_file_macro_for_sources(test_socket)
target_link_libraries(test_socket sylar ${LIB_LIB})

add_executable(test_bytearray tests/test_bytearray.cc)
add_dependencies(test_bytearray sylar)
force_redefine_file_macro_for_sources(test_bytearray)
target_link_libraries(test_bytearray sylar ${LIB_LIB})


add_executable(test_http tests/test_http.cc)
add_dependencies(test_http sylar)
force_redefine_file_macro_for_sources(test_http)
target_link_libraries(test_http sylar ${LIB_LIB})


add_executable(test_httpparser tests/test_httpparser.cc)
add_dependencies(test_httpparser sylar)
force_redefine_file_macro_for_sources(test_httpparser)
target_link_libraries(test_httpparser sylar ${LIB_LIB})


add_executable(test_tcpserver tests/test_tcpserver.cc)
add_dependencies(test_tcpserver sylar)
force_redefine_file_macro_for_sources(test_tcpserver)
target_link_libraries(test_tcpserver sylar ${LIB_LIB})


add_executable(echo_server examples/echo_server.cc)
add_dependencies(echo_server sylar)
force_redefine_file_macro_for_sources(echo_server)
target_link_libraries(echo_server sylar ${LIB_LIB})

add_executable(test_httpserver tests/test_httpserver.cc)
add_dependencies(test_httpserver sylar)
force_redefine_file_macro_for_sources(test_httpserver)
target_link_libraries(test_httpserver sylar ${LIB_LIB})

add_executable(test_httpconnection tests/test_httpconnection.cc)
add_dependencies(test_httpconnection sylar)
force_redefine_file_macro_for_sources(test_httpconnection)
target_link_libraries(test_httpconnection sylar ${LIB_LIB})


add_executable(test_uri tests/test_uri.cc)
add_dependencies(test_uri sylar)
force_redefine_file_macro_for_sources(test_uri)
target_link_libraries(test_uri sylar ${LIB_LIB})


SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)