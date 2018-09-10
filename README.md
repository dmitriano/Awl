# Awl
AWL - A Working Library

AWL is a small but constantly growing cross-platform C++ library.

Compiling on Windows with CMake and MSVC2017:

    cmake.exe ..\..\examples\lib\Awl -DCMAKE_GENERATOR_PLATFORM=x64
    msbuild AwlTest.sln /p:Configuration=Release /p:Platform=x64

Compiling on Linux with CMake and GCC 7.3:

    cmake ../../Awl/
    make -j2
