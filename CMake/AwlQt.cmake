if (NOT APPLE)
    cmake_policy(SET CMP0156 OLD)
    message(STATUS "POLICY CMP0156 has been set to OLD to avoid QT warnings.")
endif()

find_package(Qt6 COMPONENTS Core REQUIRED)

message(STATUS "Using QT6. Include path: ${Qt6_INCLUDE_DIRS}. Applying the workaround for QT Creator.")
# Sometimes QT Creator requires the configuration to be exactly Debug or Release and not RelWithDebInfo,
# so we make Release to be RelWithDebinfo with O3.
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/Zi>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-g>
)
target_compile_definitions(${PROJECT_NAME} PRIVATE AWL_QT QT_NO_EMIT)
target_include_directories(${PROJECT_NAME} PRIVATE ${Qt6_INCLUDE_DIRS} ${AWL_ROOT_DIR}/Extras/Qt)
file(GLOB_RECURSE QT_EXTTRAS_FILES ${CMAKE_SOURCE_DIR}/Extras/Qt/*.h ${AWL_ROOT_DIR}/Extras/Qt/*.cpp)
target_sources(${PROJECT_NAME} PRIVATE ${QT_EXTTRAS_FILES})
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core)
