# Awl
AWL - A Working Library

AWL is a small cross-platform C++ library that includes:

1. A simple binary [serialization framework](https://developernote.com/2020/02/a-simple-cpp-serialization-framework/).
2. Memory stream, buffered stream, hashing stream.
3. A set that finds an element by both key and index with O(logN) time.
4. A doubly linked list with static insert and erase methods.
5. A moveable observable pattern.
6. Transform iterator.
7. Bitset based on enum.
8. A simple testing framework.

Feel free to use it or fork it, but keep in mind that version compatibility is not guaranteed and there is no warranty of any kind.

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
