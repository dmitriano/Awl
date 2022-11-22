set(AWL_DIR ${AWL_ROOT_DIR}/Awl)

target_include_directories(${PROJECT_NAME} PRIVATE ${AWL_ROOT_DIR})

if (AWL_JTHREAD_EXTRAS)
    message("Using home made implementation of std::jthread.")
    target_include_directories(${PROJECT_NAME} PRIVATE ${AWL_ROOT_DIR}/Extras/JThread)
    add_definitions(-DAWL_JTHREAD_EXTRAS)
endif()

#header-only libraries have no designated component
find_package(Boost)

if(Boost_FOUND)
    message("Using BOOST. Include path: ${Boost_INCLUDE_DIRS}")
    target_include_directories(${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIRS} ${AWL_ROOT_DIR}/Extras/Boost)
    add_definitions(-DAWL_BOOST)
    file(GLOB_RECURSE BOOST_FILES ${CMAKE_SOURCE_DIR}/Extras/Boost/*.h ${AWL_ROOT_DIR}/Extras/Boost/*.cpp)
    list (APPEND CPP_FILES ${BOOST_FILES})
else()
    message("BOOST not found, AWL will compile without BOOST.")
endif()

find_package(Threads)

target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})

if (AWL_COMPILE_SOURCES)
    file(GLOB_RECURSE AWL_CPP_SOURCE_FILES ${AWL_ROOT_DIR}/Awl/*.h ${AWL_ROOT_DIR}/Awl/*.cpp)
    target_sources(${PROJECT_NAME} PRIVATE ${AWL_CPP_SOURCE_FILES})
endif()

if (AWL_COMPILE_TESTS)
    file(GLOB_RECURSE AWL_CPP_TEST_FILES ${AWL_ROOT_DIR}/Tests/*.h ${AWL_ROOT_DIR}/Tests/*.cpp)
    target_sources(${PROJECT_NAME} PRIVATE ${AWL_CPP_TEST_FILES})
endif()