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
8. A [circular buffer](https://github.com/dmitriano/Awl/blob/master/Awl/Ring.h) with an interface similar to std::queue.
9. Other simple classes like CompositeCompare, ReverseCompare, scope_guard, etc...
10. A simple [testing framework](https://github.com/dmitriano/Awl/tree/master/Awl/Testing).

Theoretically, the master branch should compile with C++20 and work, at least it is periodically built with MSVC 2022 comiler 19.30.30709+ and GCC 11, but not with CLang yet.

There is also cpp17 branch that partially compiles with C++17.

Version compatibility is not guaranteed and there is no warranty of any kind.

Feel free to use it or fork it, report a bug by opening an issue.

To leave the author a message fill the [form on his website](https://developernote.com/contact/).

Compiling on Windows with CMake and MSVC 2022:

    cmake ..\..\Awl -G "Visual Studio 17 2022" -A x64
    cmake --build . --target AwlTest --config Release

or

    msbuild AwlTest.sln /p:Configuration=Release /p:Platform=x64

Compiling on Linux with CMake and GCC:

    cmake ../../Awl/ -DCMAKE_BUILD_TYPE=Release
    cmake --build . --parallel

or

    cmake ../../Awl/ -DCMAKE_BUILD_TYPE=Debug
    cmake --build . --parallel

[Compiling on Ubuntu 18.04 and 20.04 with GCC 11](https://developernote.com/2021/08/compiling-awl-on-ubuntu-18-with-gcc-11/) (open the link).

Compiling on Ubuntu 22.04:

    sudo apt install build-essential
    sudo apt install cmake

    mkdir repos
    cd repos
    git clone https://github.com/dmitriano/Awl
    mkdir -p build/awl
    cd build/awl
    cmake ../../Awl/ -DCMAKE_BUILD_TYPE=Release
    cmake --build . --parallel

To enable GCC sanitizer uncomment corresponding lines in CMakeLists.txt.

Running the tests (remove "./" prefix on Windows and do not forget quotes on Linux):

    ./AwlTest

of

    ./AwlTest --filter ".*_Test"

Running the benchmarks:

    ./AwlTest --filter ".*_Benchmark" --output all

Running the examples:

    ./AwlTest --filter ".*_Example" --output all

Do not run the commands below:

    ./AwlTest --filter ".*_Unstable"

or

    ./AwlTest --filter ".*"

they potentially can format your hard drive.
