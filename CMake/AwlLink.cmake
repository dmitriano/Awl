set(AWL_COMPILER_GNU_OR_CLANG ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" 
    OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang"))
if (${AWL_COMPILER_GNU_OR_CLANG})
    # won't work before project()!    
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        # 64 bits
        message("Enabling 128 bit integer support.")
        add_compile_definitions(AWL_INT_128)
    endif()
    if (AWL_NO_DEPRECATED)
        add_definitions("-Wno-deprecated -Wno-deprecated-declarations")
    endif()
endif()

target_include_directories(${PROJECT_NAME} PRIVATE ${AWL_ROOT_DIR})

if (AWL_STATIC_RUNTIME)
    if (${AWL_COMPILER_GNU_OR_CLANG})
        message("Building with static runtime.")
        target_link_options(${PROJECT_NAME} PRIVATE -static-libgcc -static-libstdc++)
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Debug>:/MTd>
            $<$<NOT:$<CONFIG:Debug>>:/MT>)
    endif()
endif()

if (AWL_JTHREAD_EXTRAS)
    message(STATUS "Using home made implementation of std::jthread.")
    target_include_directories(${PROJECT_NAME} PRIVATE ${AWL_ROOT_DIR}/Extras/JThread)
    target_compile_definitions(${PROJECT_NAME} PRIVATE AWL_JTHREAD_EXTRAS)
endif()

if (WIN32)
    set(AWL_PLATFORM_DIR ${AWL_ROOT_DIR}/Platforms/Windows)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _WIN32_WINNT=0x0A00)
    message(STATUS "Win32 native platform.")
elseif (UNIX)
    set(AWL_PLATFORM_DIR ${AWL_ROOT_DIR}/Platforms/Posix)
    message(STATUS "Posix native platform.")
else()
    message(STATUS "No native platform support.")
endif()

if (DEFINED AWL_PLATFORM_DIR)
    message(STATUS "AWL_PLATFORM_DIR: ${AWL_PLATFORM_DIR}")
    target_include_directories(${PROJECT_NAME} PRIVATE ${AWL_PLATFORM_DIR})
    file(GLOB_RECURSE AWL_PLATFORM_FILES ${AWL_PLATFORM_DIR}/*.h ${AWL_PLATFORM_DIR}/*.cpp)
    target_sources(${PROJECT_NAME} PRIVATE ${AWL_PLATFORM_FILES})
    if (AWL_COMPILE_TESTS)
        file(GLOB_RECURSE AWL_PLATFORM_TEST_FILES ${AWL_ROOT_DIR}/PlatformTests/*.h ${AWL_ROOT_DIR}/PlatformTests/*.cpp)
        target_sources(${PROJECT_NAME} PRIVATE ${AWL_PLATFORM_TEST_FILES})
    endif()
endif()

if (AWL_FIND_OPENSSL)
    find_package(OpenSSL REQUIRED Crypto SSL REQUIRED)
    message(STATUS "OpenSSL include: ${OPENSSL_INCLUDE_DIR}")
    target_compile_definitions(${PROJECT_NAME} PRIVATE AWL_OPENSSL)
    include_directories(${OPENSSL_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenSSL::Crypto OpenSSL::SSL)
endif()

if (AWL_FIND_BOOST)
    #header-only libraries have no designated component
    set(Boost_USE_STATIC_LIBS ON)
    set(Boost_USE_MULTITHREADED ON)
    set(Boost_USE_STATIC_RUNTIME ON)
    find_package(Boost COMPONENTS atomic thread container exception REQUIRED)

    message(STATUS "Using BOOST. Include path: ${Boost_INCLUDE_DIRS} ${Boost_LIB_DIRS}")
    target_compile_definitions(${PROJECT_NAME} PRIVATE AWL_BOOST)
    target_include_directories(${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIRS} ${AWL_ROOT_DIR}/Extras/Boost)
    file(GLOB_RECURSE BOOST_FILES ${CMAKE_SOURCE_DIR}/Extras/Boost/*.h ${AWL_ROOT_DIR}/Extras/Boost/*.cpp)
    target_sources(${PROJECT_NAME} PRIVATE ${BOOST_FILES})
    target_link_libraries(${PROJECT_NAME} PRIVATE crypt32 ${Boost_LIBRARIES})
    message(STATUS "BOOST libs: ${Boost_LIBRARIES} ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY}.")
endif()

if (AWL_FIND_QT)
    # Check if the project is built with QT.
    find_package(Qt6 COMPONENTS Core REQUIRED)

    message(STATUS "Using QT6. Include path: ${Qt6_INCLUDE_DIRS}. Applying the workaround for QT Creator.")
    # Sometimes QT Creator requires the configuration to be exactly Debug or Release and not RelWithDebInfo,
    # so we make Release to be RelWithDebinfo with O3.
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/Zi>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-g>
    )
    target_compile_definitions(${PROJECT_NAME} PRIVATE AWL_QT)
    target_include_directories(${PROJECT_NAME} PRIVATE ${Qt6_INCLUDE_DIRS} ${AWL_ROOT_DIR}/Extras/Qt)
    file(GLOB_RECURSE QT_EXTTRAS_FILES ${CMAKE_SOURCE_DIR}/Extras/Qt/*.h ${AWL_ROOT_DIR}/Extras/Qt/*.cpp)
    target_sources(${PROJECT_NAME} PRIVATE ${QT_EXTTRAS_FILES})
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core)
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

if (AWL_ANSI_CMD_CHAR)
    target_compile_definitions(${PROJECT_NAME} PRIVATE AWL_ANSI_CMD_CHAR)
endif()
