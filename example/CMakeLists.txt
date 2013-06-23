cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project(kobuki_sample)

find_package(Threads)

if(NOT WIN32)
find_package(kobuki)
endif(NOT WIN32)

if(WIN32)
set(LIBKOBUKI_DIR $ENV{LIBKOBUKI_ROOT})
set(kobuki_INCLUDE_DIRS ${LIBKOBUKI_DIR}/include)
set(kobuki_LIBRARY_DIRS ${LIBKOBUKI_DIR}/lib)
set(kobuki_LIBRARIES kobuki)
endif(WIN32)


set(PROJECT_VERSION 1.0.0 CACHE STRING "kobuki_sample")
set(PROJECT_VENDOR "RT_net")
set(PROJECT_AUTHOR "RT_net")
set(PROJECT_AUTHOR_SHORT "RT_net")

set(_srcs demo.cpp)

include_directories(${kobuki_INCLUDE_DIRS})
link_directories(${kobuki_LIBRARY_DIRS})

add_executable(kobuki_sample ${_srcs})
target_link_libraries(kobuki_sample kobuki ${CMAKE_THREAD_LIBS_INIT})

