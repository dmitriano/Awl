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
target_link_libraries(${PROJECT_NAME} PRIVATE ${Boost_LIBRARIES})
message(STATUS "BOOST libs: ${Boost_LIBRARIES} ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY}.")

if (WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE crypt32)
endif()
