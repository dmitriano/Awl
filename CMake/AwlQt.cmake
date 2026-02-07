# I tried to prevent the following warning from QT6 find package:

#   CMP0156 is set to 'NEW'.  Qt forces the 'OLD' behavior of this policy for
#   non-Apple platforms by default.  Set QT_FORCE_CMP0156_TO_VALUE=NEW to force
#   the 'NEW' behavior for Qt commands that create library or executable
#   targets.

# with the following code:

# if (NOT APPLE)
#     cmake_policy(SET CMP0156 OLD)
#     message(STATUS "POLICY CMP0156 has been set to OLD to avoid QT warnings.")
# endif()

# but it did not help

find_package(Qt6 COMPONENTS Core REQUIRED)

# Suppress "Qt policy QTP0002 is not set" warning.
qt_policy(SET QTP0002 NEW)

message(STATUS "Using QT6. Include path: ${Qt6_INCLUDE_DIRS}. Applying the workaround for QT Creator.")
# Sometimes QT Creator requires the configuration to be exactly Debug or Release and not RelWithDebInfo,
# so we make Release to be RelWithDebinfo with O3.
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/Zi>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-g>
)
target_compile_definitions(${PROJECT_NAME} PRIVATE AWL_QT QT_NO_EMIT)
target_include_directories(${PROJECT_NAME} PRIVATE ${Qt6_INCLUDE_DIRS} ${AWL_ROOT_DIR}/Extras/Qt)
file(GLOB_RECURSE QT_EXTTRAS_FILES ${AWL_ROOT_DIR}/Extras/Qt/QtExtras/*.h ${AWL_ROOT_DIR}/Extras/Qt/QtExtras/*.cpp)
target_sources(${PROJECT_NAME} PRIVATE ${QT_EXTTRAS_FILES})

if (AWL_COMPILE_TESTS)
    file(GLOB_RECURSE QT_EXTTRAS_TEST_FILES ${AWL_ROOT_DIR}/Extras/Qt/Tests/*.h ${AWL_ROOT_DIR}/Extras/Qt/Tests/*.cpp)
    target_sources(${PROJECT_NAME} PRIVATE ${QT_EXTTRAS_TEST_FILES})
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core)
