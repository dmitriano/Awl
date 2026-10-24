cmake_minimum_required(VERSION 3.24.2...4.4.3)

project(Awl LANGUAGES CXX)

include(CMake/AwlConfig.cmake)
include(CMake/AwlTargets.cmake)
awl_add_libraries()

option(AWL_BUILD_TESTS "Build the standalone AWL test executable" ${PROJECT_IS_TOP_LEVEL})
if(AWL_BUILD_TESTS)
    add_executable(AwlTest)
    awl_add_test_sources(AwlTest)
    target_link_libraries(AwlTest PRIVATE AWL::TestMain)
    awl_apply_compiler_options(AwlTest)
endif()
