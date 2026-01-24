cmake_minimum_required(VERSION 3.24.2...4.1.2)

message(STATUS "CMake version: ${CMAKE_VERSION}")
message(STATUS "CMAKE_SYSTEM_VERSION: ${CMAKE_SYSTEM_VERSION}")
message(STATUS "AwlConfig.cmake is in: ${CMAKE_CURRENT_LIST_DIR} directory.")

cmake_path(GET CMAKE_CURRENT_LIST_DIR PARENT_PATH AWL_ROOT_DIR)
message(STATUS "AWL_ROOT_DIR: ${AWL_ROOT_DIR}")
set(AWL_DIR ${AWL_ROOT_DIR}/Awl)

if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
    message(STATUS "POLICY CMP0167 has been set to NEW to prevent BOOST warnings.")
endif()

if (NOT APPLE)
    cmake_policy(SET CMP0156 OLD)
    message(STATUS "POLICY CMP0156 has been set to OLD to avoid QT warnings.")
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
option(AWL_DEBUG_IMMUTABLE "Enable debug mode for immutable.")
option(AWL_SANITIZE_THREAD "Use Thread Sanitizer.")
option(AWL_SANITIZE_UNDEFINED "Use Undefined Behavior Sanitizer.")
option(AWL_SANITIZE_ADDRESS "Use Address Sanitizer.")
