cmake_minimum_required(VERSION 3.24.2...4.1.2)

message(STATUS "CMake version: ${CMAKE_VERSION}")
message(STATUS "CMAKE_SYSTEM_VERSION: ${CMAKE_SYSTEM_VERSION}")
message(STATUS "AwlConfig.cmake is in: ${CMAKE_CURRENT_LIST_DIR} directory.")

cmake_path(GET CMAKE_CURRENT_LIST_DIR PARENT_PATH AWL_ROOT_DIR)
message(STATUS "AWL_ROOT_DIR: ${AWL_ROOT_DIR}")

# Prevent BOOST warnings.
if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
    message(STATUS "POLICY CMP0167 has been set.")
endif()

set(CMAKE_CXX_STANDARD 23)

message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")

option(AWL_JTHREAD_EXTRAS "Use home made implementation of std::jthread.")
option(AWL_FIND_OPENSSL "Use OpenSSL.")
option(AWL_FIND_QT "Use QString if the project is built with QT.")
option(AWL_FIND_BOOST "Use boost::multiprecision.")
option(AWL_COMPILE_SOURCES "Compile AWL sources." ON)
option(AWL_COMPILE_TESTS "Compile AWL tests.")
option(AWL_COMPILE_EXPERIMENTAL "Compile experimental AWL code.")
option(AWL_COMPILE_MAIN "Compile and link AWL main() function.")
option(AWL_NO_DEPRECATED "Disable deprecated warnings in C++.")
option(AWL_STATIC_RUNTIME "Linking statically against libgcc and libstdc++.")
option(AWL_ANSI_CMD_CHAR "Define CommandLineProvider with char, but not with awl::Char.")
option(AWL_SANITIZE_THREAD "Use Thread Sanitizer.")
option(AWL_SANITIZE_UNDEFINED "Use Undefined Behavior Sanitizer.")
option(AWL_SANITIZE_ADDRESS "Use Address Sanitizer.")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    # Apple Clang does not have std::jthread.
    # set(AWL_JTHREAD_EXTRAS ON)
    add_definitions("-fexperimental-library")
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "20.0.0")
        set(AWL_JTHREAD_EXTRAS ON)
    else()
        # Android Clang has std::jthread since 20.0 as experimental.
        add_definitions("-fexperimental-library")
    endif()
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_definitions("-Wall -Wextra -pedantic")
    # Unused operators in local namespaces defined by AWL_MEMBERWISE_EQUATABLE
    add_definitions("-Wno-unused-function")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_definitions("-Wall -Wextra -pedantic")
    #add_definitions("-Wall -Wextra -pedantic -pthread")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
    # using Intel C++
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    # using Visual Studio C++
    # add_compile_options("/std:c++latest")
    add_compile_options("/W4" "/Zc:__cplusplus")
    add_definitions(-MP -D_UNICODE -D_CRT_SECURE_NO_WARNINGS -DNOMINMAX)
    if (AWL_NO_DEPRECATED)
        add_definitions(-D_SILENCE_ALL_CXX23_DEPRECATION_WARNINGS)
    endif()
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    if (AWL_SANITIZE_THREAD)
        message(STATUS "Using Thread Sanitizer.")
        add_definitions("-fsanitize=thread -fno-omit-frame-pointer")
        link_libraries("-fsanitize=thread")
    endif()
    if (AWL_SANITIZE_UNDEFINED)
        message(STATUS "Using Undefined Behavior Sanitizer.")
        add_definitions("-fsanitize=undefined -fno-omit-frame-pointer")
        link_libraries("-fsanitize=undefined")
    endif()
    if (AWL_SANITIZE_ADDRESS)
        message(STATUS "Using Address Sanitizer.")
        add_definitions("-fsanitize=address -fno-omit-frame-pointer")
        link_libraries("-fsanitize=address")
    endif()
    #set(CMAKE_EXE_LINKER_FLAGS "-fuse-ld=gold")
endif()

set(AWL_DIR ${AWL_ROOT_DIR}/Awl)
