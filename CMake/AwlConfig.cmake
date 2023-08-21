cmake_minimum_required (VERSION 3.6.2)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(CMAKE_CXX_STANDARD 23)
else()
    set(CMAKE_CXX_STANDARD 20)
endif()

option(AWL_JTHREAD_EXTRAS "Use home made implementation of std::jthread.")
option(AWL_COMPILE_SOURCES "Compile AWL sources." ON)
option(AWL_COMPILE_TESTS "Compile AWL tests.")
option(AWL_COMPILE_EXPERIMENTAL "Compile experimental AWL code.")
option(AWL_COMPILE_MAIN "Compile and link AWL main() function.")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    message("Enabling 128 bit integer support.")
    add_compile_definitions(AWL_INT_128)
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    #AppleClang and Android Clang do not have std::jthread.
    set(AWL_JTHREAD_EXTRAS ON)
    add_definitions("-Wall -Wextra -pedantic")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_definitions("-Wall -Wextra -pedantic")
    #add_definitions("-Wall -Wextra -pedantic -pthread")
    #add_definitions("-fsanitize=thread -fno-omit-frame-pointer")
    #link_libraries("-fsanitize=thread")
    #add_definitions("-fsanitize=undefined -fno-omit-frame-pointer")
    #link_libraries("-fsanitize=undefined")
    #add_definitions("-fsanitize=address -fno-omit-frame-pointer")
    #link_libraries("-fsanitize=address")
    #set(CMAKE_EXE_LINKER_FLAGS "-fuse-ld=gold")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
    # using Intel C++
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    # using Visual Studio C++
    # add_compile_options("/std:c++latest")
    add_compile_options("/W4" "/Zc:__cplusplus")
    add_definitions(-MP -D_UNICODE -D_CRT_SECURE_NO_WARNINGS -DNOMINMAX)
endif()

set(AWL_DIR ${AWL_ROOT_DIR}/Awl)
