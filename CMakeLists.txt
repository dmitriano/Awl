cmake_minimum_required (VERSION 3.6.2)
project (AwlTest)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

if (MSVC_VERSION GREATER_EQUAL "1900")
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("/std:c++latest" _cpp_latest_flag_supported)
    if (_cpp_latest_flag_supported)
        add_compile_options("/std:c++latest")
    endif()
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  # using Clang
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  # using GCC
  add_definitions("-std=c++14 -Wall -Wextra -pedantic -pthread")
  #Enable multithreading to use std::thread: Operation not permitted
  #http://stackoverflow.com/questions/30734136/cmake-linking-error-pthread-enable-multithreading-to-use-stdthread-operation
  #SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++11 -pthread")

  #add_definitions("-fsanitize=thread -fno-omit-frame-pointer")
  #link_libraries("-fsanitize=thread")
  #add_definitions("-fsanitize=undefined -fno-omit-frame-pointer")
  #link_libraries("-fsanitize=undefined")
  add_definitions("-fsanitize=address -fno-omit-frame-pointer")
  link_libraries("-fsanitize=address")
  set(CMAKE_EXE_LINKER_FLAGS "-fuse-ld=gold")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  # using Intel C++
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  # using Visual Studio C++
  add_compile_options("/W4")
  add_definitions(-D_UNICODE)
endif()

find_package (Threads)

file(GLOB_RECURSE CPP_FILES ${CMAKE_SOURCE_DIR}/*.h ${CMAKE_SOURCE_DIR}/*.cpp)

add_executable(${PROJECT_NAME} ${CPP_FILES})

target_link_libraries (${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})
