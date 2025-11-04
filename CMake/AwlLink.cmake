cmake_minimum_required(VERSION 3.24.2...4.1.2)

include(${AWL_ROOT_DIR}/CMake/AwlCompilerOptions.cmake)

target_include_directories(${PROJECT_NAME} PRIVATE ${AWL_ROOT_DIR})

if (AWL_JTHREAD_EXTRAS)
    message(STATUS "Using home made implementation of std::jthread.")
    target_include_directories(${PROJECT_NAME} PRIVATE ${AWL_ROOT_DIR}/Extras/JThread)
    target_compile_definitions(${PROJECT_NAME} PRIVATE AWL_JTHREAD_EXTRAS)
endif()

include(${AWL_ROOT_DIR}/CMake/AwlNativePlatform.cmake)

if (AWL_FIND_OPENSSL)
    include(${AWL_ROOT_DIR}/CMake/AwlOpenSsl.cmake)
endif()

if (AWL_FIND_BOOST)
    include(${AWL_ROOT_DIR}/CMake/AwlBoost.cmake)
endif()

if (AWL_FIND_QT)
    include(${AWL_ROOT_DIR}/CMake/AwlQt.cmake)
endif()

include(${AWL_ROOT_DIR}/CMake/AwlThreads.cmake)

if (AWL_COMPILE_SOURCES)
    file(GLOB_RECURSE AWL_CPP_SOURCE_FILES ${AWL_ROOT_DIR}/Awl/*.h ${AWL_ROOT_DIR}/Awl/*.cpp)
    target_sources(${PROJECT_NAME} PRIVATE ${AWL_CPP_SOURCE_FILES})
endif()

if (AWL_COMPILE_TESTS)
    file(GLOB_RECURSE AWL_CPP_TEST_FILES ${AWL_ROOT_DIR}/Tests/*.h ${AWL_ROOT_DIR}/Tests/*.cpp)
    target_sources(${PROJECT_NAME} PRIVATE ${AWL_CPP_TEST_FILES})
endif()

if (AWL_COMPILE_MAIN)
    file(GLOB_RECURSE AWL_CPP_MAIN_FILES ${AWL_ROOT_DIR}/Main/*.h ${AWL_ROOT_DIR}/Main/*.cpp)
    target_sources(${PROJECT_NAME} PRIVATE ${AWL_CPP_MAIN_FILES})
endif()

if (AWL_ANSI_CMD_CHAR)
    target_compile_definitions(${PROJECT_NAME} PRIVATE AWL_ANSI_CMD_CHAR)
endif()
