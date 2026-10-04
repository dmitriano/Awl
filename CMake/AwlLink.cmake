cmake_minimum_required(VERSION 3.24.2...4.4.3)

# Compatibility with existing include(AwlLink.cmake) consumers.
# Implementations belong to reusable libraries; MAIN/TESTS remain per consumer.
include("${CMAKE_CURRENT_LIST_DIR}/AwlTargets.cmake")
awl_link(${PROJECT_NAME})
