cmake_minimum_required (VERSION 3.9.1)
project (AwlTest)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  # using Clang
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  # using GCC
  add_definitions("-std=c++11 -Wall -Wextra -pedantic -pthread")
  #Enable multithreading to use std::thread: Operation not permitted
  #http://stackoverflow.com/questions/30734136/cmake-linking-error-pthread-enable-multithreading-to-use-stdthread-operation
  SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++11 -pthread")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  # using Intel C++
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  # using Visual Studio C++
  add_definitions(-D_UNICODE)
endif()

find_package (Threads)

file(GLOB_RECURSE CPP_FILES ${CMAKE_SOURCE_DIR}/*.h ${CMAKE_SOURCE_DIR}/*.cpp)

add_executable(${PROJECT_NAME} ${CPP_FILES})

target_link_libraries (${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})
