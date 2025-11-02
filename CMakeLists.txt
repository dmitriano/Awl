cmake_minimum_required(VERSION 3.24.2...4.1.2)

project(AwlTest)

include(CMake/AwlConfig.cmake)

add_executable(${PROJECT_NAME})

set(AWL_COMPILE_TESTS ON)
set(AWL_COMPILE_MAIN ON)

include(CMake/AwlLink.cmake)
