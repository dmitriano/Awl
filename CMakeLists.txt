cmake_minimum_required (VERSION 3.6.2)

project(AwlTest)

include(CMake/AwlConfig.cmake)

add_executable(${PROJECT_NAME})

message("Awl C++ standard: ${CMAKE_CXX_STANDARD}")

#${CMAKE_SOURCE_DIR}
set(AWL_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(AWL_COMPILE_TESTS ON)
set(AWL_COMPILE_MAIN ON)

include(CMake/AwlLink.cmake)
