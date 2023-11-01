target_include_directories(${PROJECT_NAME} PRIVATE ${AWL_ROOT_DIR})

if (AWL_STATIC_RUNTIME)
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        target_link_options(${PROJECT_NAME} PRIVATE -static-libgcc -static-libstdc++)
    endif()
endif()

if (AWL_JTHREAD_EXTRAS)
    message("Using home made implementation of std::jthread.")
    target_include_directories(${PROJECT_NAME} PRIVATE ${AWL_ROOT_DIR}/Extras/JThread)
    add_definitions(-DAWL_JTHREAD_EXTRAS)
endif()

message("CMAKE_SYSTEM_VERSION: ${CMAKE_SYSTEM_VERSION}")

if (WIN32)
    set(AWL_PLATFORM_DIR ${AWL_ROOT_DIR}/Platforms/Windows)
    message("Win32 native platform.")
elseif (UNIX)
    set(AWL_PLATFORM_DIR ${AWL_ROOT_DIR}/Platforms/Posix)
    message("Posix native platform.")
else()
    message("No native platform support.")
endif()

if (DEFINED AWL_PLATFORM_DIR)
    message("AWL_PLATFORM_DIR: ${AWL_PLATFORM_DIR}")
    target_include_directories(${PROJECT_NAME} PRIVATE ${AWL_PLATFORM_DIR})
    file(GLOB_RECURSE AWL_PLATFORM_FILES ${AWL_PLATFORM_DIR}/*.h ${AWL_PLATFORM_DIR}/*.cpp)
    target_sources(${PROJECT_NAME} PRIVATE ${AWL_PLATFORM_FILES})
    if (AWL_COMPILE_TESTS)
        file(GLOB_RECURSE AWL_PLATFORM_TEST_FILES ${AWL_ROOT_DIR}/PlatformTests/*.h ${AWL_ROOT_DIR}/PlatformTests/*.cpp)
        target_sources(${PROJECT_NAME} PRIVATE ${AWL_PLATFORM_TEST_FILES})
    endif()
endif()

#header-only libraries have no designated component
find_package(Boost)

if(Boost_FOUND)
    message("Using BOOST. Include path: ${Boost_INCLUDE_DIRS}")
    target_include_directories(${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIRS} ${AWL_ROOT_DIR}/Extras/Boost)
    add_definitions(-DAWL_BOOST)
    file(GLOB_RECURSE BOOST_FILES ${CMAKE_SOURCE_DIR}/Extras/Boost/*.h ${AWL_ROOT_DIR}/Extras/Boost/*.cpp)
    target_sources(${PROJECT_NAME} PRIVATE ${BOOST_FILES})
else()
    message("BOOST not found, AWL will compile without BOOST.")
endif()

# Check if the project is built with QT.
find_package(Qt6)

if(Qt6_FOUND)
    message("Applying the workaround for QT Creator.")
    # Sometimes QT Creator requires the configuration to be exactly Debug or Release and not RelWithDebInfo,
    # so we make Release to be RelWithDebinfo with O3.
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/Zi>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-g>
    )
endif()

find_package(Threads)

target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)

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
