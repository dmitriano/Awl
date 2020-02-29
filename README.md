# Awl
AWL - A Working Library

AWL is a small cross-platform C++ library that includes:

1. A simple binary serialization framework.
2. A set that finds an element by both key and index with O(logN) time.
3. A doubly linked list with static insert and erase methods.
4. A moveable observable pattern.
5. Transform iterator.
6. Bitset based on enum.
7. A simple testing framework.

Compiling on Windows with CMake and MSVC2017:

    cmake.exe ..\..\Awl -DCMAKE_GENERATOR_PLATFORM=x64
    msbuild AwlTest.sln /p:Configuration=Release /p:Platform=x64

Compiling on Linux with CMake and GCC 9:

    cmake ../../Awl/ -DCMAKE_BUILD_TYPE=Release
    make -j2

Running the tests:

    AwlTest --filter .*Test

Running the benchmarks:

    AwlTest --filter .*Benchmark --output all
