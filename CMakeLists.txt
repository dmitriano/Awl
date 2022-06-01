cmake_minimum_required (VERSION 3.6.2)
project (AwlTest)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  set(CMAKE_CXX_STANDARD 23)
else()
  set(CMAKE_CXX_STANDARD 20)
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  # using Clang
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  # using GCC
  add_definitions("-Wall -Wextra -Wno-unused-function -pedantic -pthread")
  #usign GCC sanitizer:
  #add_definitions("-fsanitize=thread -fno-omit-frame-pointer")
  #link_libraries("-fsanitize=thread")
  #add_definitions("-fsanitize=undefined -fno-omit-frame-pointer")
  #link_libraries("-fsanitize=undefined")
  #add_definitions("-fsanitize=address -fno-omit-frame-pointer")
  #link_libraries("-fsanitize=address")
  #set(CMAKE_EXE_LINKER_FLAGS "-fuse-ld=gold")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  # using Intel C++
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  # using Visual Studio C++
  # add_compile_options("/std:c++latest")
  add_compile_options("/W4" "/Zc:__cplusplus")
  add_definitions(-MP -D_UNICODE)
endif()

find_package(Threads)

file(GLOB_RECURSE CPP_FILES ${CMAKE_SOURCE_DIR}/Awl/*.h ${CMAKE_SOURCE_DIR}/Awl/*.cpp
	${CMAKE_SOURCE_DIR}/Tests/*.h ${CMAKE_SOURCE_DIR}/Tests/*.cpp)

IF(DEFINED ENV{BOOST_SRC_DIR})
	message("Boost found in $ENV{BOOST_SRC_DIR}")
	include_directories(ENV{BOOST_SRC_DIR} ${CMAKE_SOURCE_DIR}/Extras/Boost)
	add_definitions(-DAWL_BOOST)
	file(GLOB_RECURSE BOOST_FILES
		${CMAKE_SOURCE_DIR}/Extras/Boost/*.h ${CMAKE_SOURCE_DIR}/Extras/Boost/*.cpp)
	list (APPEND CPP_FILES ${BOOST_FILES})
ELSE()
	message("BOOST_SRC_DIR is not defined, AWL will compile without BOOST.")
ENDIF()

add_executable(${PROJECT_NAME} ${CPP_FILES})

target_link_libraries (${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})
