cmake_minimum_required(VERSION 3.28.3...4.1.2)

project(AwlTest)

include(CMake/AwlConfig.cmake)

add_executable(${PROJECT_NAME})

#${CMAKE_SOURCE_DIR}
set(AWL_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(AWL_COMPILE_TESTS ON)
set(AWL_COMPILE_MAIN ON)

include(CMake/AwlLink.cmake)
