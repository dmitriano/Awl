# Awl
AWL - A Working Library

AWL is a small cross-platform C++ library that includes:

1. A simple binary [serialization framework](https://developernote.com/2020/02/a-simple-cpp-serialization-framework/).
2. Memory stream, buffered stream, hashing stream.
3. A [set](https://github.com/dmitriano/Awl/blob/master/Awl/VectorSet.h) that finds an element by both key and index with O(logN) time.
4. A [doubly linked list](https://github.com/dmitriano/Awl/blob/master/Awl/QuickList.h) with static insert and erase methods.
5. An [observable](https://github.com/dmitriano/Awl/blob/master/Awl/Observable.h) with movable observers.
6. A [pool of reusable objects](https://github.com/dmitriano/Awl/blob/master/Awl/ObjectPool.h) managed with std::shared_ptr.
7. [Bitset based on enum](https://github.com/dmitriano/Awl/blob/master/Awl/BitMap.h).
8. A [circular buffer](https://github.com/dmitriano/Awl/blob/master/Awl/Ring.h) with an interface similar to std::queue.
9. And other simple classes like CompositeCompare, ReverseCompare, ScopeGuard, etc...
10. A simple [testing framework](https://github.com/dmitriano/Awl/tree/master/Awl/Testing).

Feel free to use it or fork it, but keep in mind that version compatibility is not guaranteed and there is no warranty of any kind.

Theoretically, the master branch should compile and work with C++20.

Compiling on Windows with CMake and MSVC:

    cmake.exe ..\..\Awl -DCMAKE_GENERATOR_PLATFORM=x64
    set CL=/MP
    msbuild AwlTest.sln /p:Configuration=Release /p:Platform=x64

Compiling on Linux with CMake and GCC:

    cmake ../../Awl/ -DCMAKE_BUILD_TYPE=Release
    make -j2

Running the tests:

    AwlTest --filter .*Test

Running the benchmarks:

    AwlTest --filter .*Benchmark --output all
