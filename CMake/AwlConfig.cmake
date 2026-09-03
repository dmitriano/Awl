cmake_minimum_required(VERSION 3.24.2...4.1.2)

message(STATUS "CMake version: ${CMAKE_VERSION}")
message(STATUS "CMAKE_SYSTEM_VERSION: ${CMAKE_SYSTEM_VERSION}")
message(STATUS "AwlConfig.cmake is in: ${CMAKE_CURRENT_LIST_DIR} directory.")

cmake_path(GET CMAKE_CURRENT_LIST_DIR PARENT_PATH AWL_ROOT_DIR)
message(STATUS "AWL_ROOT_DIR: ${AWL_ROOT_DIR}")
set(AWL_DIR ${AWL_ROOT_DIR}/Awl)

set(CMAKE_CXX_STANDARD 23)

message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")

option(AWL_PARALLEL_BUILD "Enable parallel build with MSVC." ON)
option(AWL_JTHREAD_EXTRAS "Use home made implementation of std::jthread.")

set(AWL_HAS_STD_MOVE_ONLY_FUNCTION_DEFAULT ON)

# Android and Apple standard libraries do not support std::move_only_function yet.
if (ANDROID OR APPLE)
    set(AWL_HAS_STD_MOVE_ONLY_FUNCTION_DEFAULT OFF)
endif()

option(AWL_HAS_STD_MOVE_ONLY_FUNCTION
    "The standard library provides std::move_only_function."
    ${AWL_HAS_STD_MOVE_ONLY_FUNCTION_DEFAULT})

unset(AWL_HAS_STD_MOVE_ONLY_FUNCTION_DEFAULT)

option(AWL_FIND_OPENSSL "Use OpenSSL.")
option(AWL_FIND_QT "Use QString if the project is built with QT.")
option(AWL_FIND_BOOST "Use boost::multiprecision.")
option(AWL_COMPILE_SOURCES "Compile AWL sources." ON)
option(AWL_COMPILE_TESTS "Compile AWL tests.")
option(AWL_COMPILE_EXPERIMENTAL "Compile experimental AWL code.")
option(AWL_COMPILE_MAIN "Compile and link AWL main() function.")
option(AWL_NO_DEPRECATED "Disable deprecated warnings in C++.")
option(AWL_STATIC_RUNTIME "Linking statically against libgcc and libstdc++.")
option(AWL_BIGOBJ "Prevent errors like 'number of sections exceeded object file format limit'")
option(AWL_ANSI_CMD_CHAR "Define CommandLineProvider with char, but not with awl::Char.")
option(AWL_DEBUG_IMMUTABLE "Enable debug mode for immutable.")
option(AWL_SANITIZE_THREAD "Use Thread Sanitizer.")
option(AWL_SANITIZE_UNDEFINED "Use Undefined Behavior Sanitizer.")
option(AWL_SANITIZE_ADDRESS "Use Address Sanitizer.")

set(AWL_COMPILER_GNU_OR_CLANG ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang"
    OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang"))

# AWL_STATIC_RUNTIME is applied globally here because some external targets can
# be created before AwlCompilerOptions.cmake is included for ${PROJECT_NAME}.
# In that case target-level runtime settings would be too late to affect
# subprojects such as qtkeychain.
if (AWL_STATIC_RUNTIME)
    message(STATUS "Building with static runtime.")
    if (${AWL_COMPILER_GNU_OR_CLANG})
        add_link_options(-static-libgcc -static-libstdc++)
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()
endif()
