if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    # Apple Clang does not have std::jthread.
    # set(AWL_JTHREAD_EXTRAS ON)
    target_compile_options(${PROJECT_NAME} PRIVATE "-fexperimental-library")
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "20.0.0")
        set(AWL_JTHREAD_EXTRAS ON)
    else()
        # Android Clang has std::jthread since 20.0 as experimental.
        target_compile_options(${PROJECT_NAME} PRIVATE "-fexperimental-library")
    endif()
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE "-Wall -Wextra -pedantic")
    # Unused operators in local namespaces defined by AWL_MEMBERWISE_EQUATABLE
    target_compile_options(${PROJECT_NAME} PRIVATE "-Wno-unused-function")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_compile_options(${PROJECT_NAME} PRIVATE "-Wall -Wextra -pedantic")
    #target_compile_options(${PROJECT_NAME} PRIVATE "-Wall -Wextra -pedantic -pthread")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
    # using Intel C++
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    # using Visual Studio C++
    # target_compile_options(${PROJECT_NAME} PRIVATE "/std:c++latest")
    target_compile_options(${PROJECT_NAME} PRIVATE "/W4" "/Zc:__cplusplus")
    target_compile_options(${PROJECT_NAME} PRIVATE -MP -D_UNICODE -D_CRT_SECURE_NO_WARNINGS -DNOMINMAX)
    if (AWL_NO_DEPRECATED)
        target_compile_options(${PROJECT_NAME} PRIVATE -D_SILENCE_ALL_CXX23_DEPRECATION_WARNINGS)
    endif()
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    if (AWL_SANITIZE_THREAD)
        message(STATUS "Using Thread Sanitizer.")
        target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=thread -fno-omit-frame-pointer)
        target_link_options(${PROJECT_NAME} PRIVATE -fsanitize=thread)
    endif()
    if (AWL_SANITIZE_UNDEFINED)
        message(STATUS "Using Undefined Behavior Sanitizer.")
        target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=undefined -fno-omit-frame-pointer)
        target_link_options(${PROJECT_NAME} PRIVATE -fsanitize=undefined)
    endif()
    if (AWL_SANITIZE_ADDRESS)
        message(STATUS "Using Address Sanitizer.")
        target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=address -fno-omit-frame-pointer)
        target_link_options(${PROJECT_NAME} PRIVATE -fsanitize=address)
    endif()
    #set(CMAKE_EXE_LINKER_FLAGS "-fuse-ld=gold")
endif()

set(AWL_COMPILER_GNU_OR_CLANG ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" 
    OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang"))

if (${AWL_COMPILER_GNU_OR_CLANG})
    # won't work before project()!    
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        # 64 bits
        message(STATUS "Enabling 128 bit integer support.")
        target_compile_definitions(${PROJECT_NAME} PRIVATE AWL_INT_128)
    endif()
    if (AWL_NO_DEPRECATED)
        target_compile_options(${PROJECT_NAME} PRIVATE "-Wno-deprecated -Wno-deprecated-declarations")
    endif()
endif()

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

if (AWL_BIGOBJ)
    message(STATUS "Enabling BIGOBJ.")
    if (MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE /bigobj)
    else()
        target_compile_options(${PROJECT_NAME} PRIVATE -Wa,-mbig-obj)
    endif()
endif()

if (NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    # Use home made std::formatter for std::thread::id.
    target_compile_definitions(${PROJECT_NAME} PRIVATE AWL_THREAD_ID_FORMATTER)
endif()
