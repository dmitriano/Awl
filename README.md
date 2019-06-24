# Awl
AWL - A Working Library

AWL is a small but constantly growing cross-platform C++ library.

Compiling on Windows with CMake and MSVC2017:

    cmake.exe ..\..\Awl -DCMAKE_GENERATOR_PLATFORM=x64
    msbuild AwlTest.sln /p:Configuration=Release /p:Platform=x64

Compiling on Linux with CMake and GCC 9:

    cmake ../../Awl/ -DCMAKE_BUILD_TYPE=Release
    make -j2

Running the tests:

    AwlTest --filter .*Test

Running the benchmarks:

    AwlTest --filter .*Benchmark --verbose
