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

Theoretically, the master branch should compile with C++20 and work, at least it is periodically built with `MSVC 19.39.33523`, `GCC 13.1.0`, `Android CLang 17.0.2` and `Apple Clang 15.0.0`.

There is also cpp17 branch that partially compiles with C++17.

Version compatibility is not guaranteed and there is no warranty of any kind.

Feel free to use it or fork it, report a bug by opening an issue.

To leave the author a message fill the [form on his website](https://developernote.com/contact/).

## Compiling on Windows with CMake and MSVC 2022:

```bat
cmake ..\..\Awl -G "Visual Studio 17 2022" -A x64
cmake --build . --target AwlTest --config Release
```

or

```bat
msbuild AwlTest.sln /p:Configuration=Release /p:Platform=x64
```

It also builds for x86 using the following command:

```bat
cmake ..\..\Awl -G "Visual Studio 17 2022" -A win32
```

but with couple warnings related to std::streamsize that are not fixed yet.

## Compiling on Linux with CMake and GCC:

```bash
cmake ../../Awl/ -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel
```

or

```bash
cmake ../../Awl/ -DCMAKE_BUILD_TYPE=Debug
cmake --build . --parallel
```

## Compiling on Ubuntu 22.04 with GCC13.1 (I am not sure if build-essential is really needed):

```bash
sudo apt install build-essential
sudo apt install cmake

sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt update
sudo apt install gcc-13 g++-13
ll /usr/bin/gcc-13
ll /usr/bin/g++-13
update-alternatives --display gcc
ll /etc/alternatives/g*
sudo update-alternatives --remove-all gcc
sudo update-alternatives --remove-all g++
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 10 --slave /usr/bin/g++ g++ /usr/bin/g++-13
g++ --version
gcc --version

mkdir repos
cd repos
git clone https://github.com/dmitriano/Awl
mkdir -p build/awl
cd build/awl
cmake ../../Awl/ -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel
```

## Compiling with Ninja generator

```bash
cmake ../../repos/Awl/ -G Ninja
cmake --build . --parallel --target AwlTest --config RelWithDebInfo
```

Compiling a separate source file (by example of `VtsTest.cpp`):

```bash
cmake --build . --parallel --target CMakeFiles/AwlTest.dir/Tests/VtsTest.cpp.o
```

## Using GCC sanitizer

To enable GCC sanitizer uncomment corresponding lines in `CMake\AwlConfig.cmake`.

## Running the tests on Windows and Linux

Remove `./` prefix on Windows and do not forget quotes on Linux:

```bash
./AwlTest
```

of

```bash
./AwlTest --filter ".*_Test"
```

Running the benchmarks:

```bash
./AwlTest --filter ".*_Benchmark" --output all
```

Running the examples:

```bash
./AwlTest --filter ".*_Example" --output all
```

Do not run the commands below:

```bash
./AwlTest --filter ".*_Unstable"
```

or

```bash
./AwlTest --filter ".*"
```

they potentially can format your hard drive.

## Running the tests on Android device

Built AWL for Android with `-DAWL_STATIC_RUNTIME:BOOL=ON` CMake option, upload the executable file to the device:

```bash
adb push AwlTest /data/local/tmp
```

and run it with the same options as on Linux:

```bash
adb shell
cd /data/local/tmp
chmod a+x AwlTest
./AwlTest
```

or with a single command:

```bash
adb shell "cd /data/local/tmp && chmod a+x AwlTest && ./AwlTest"
```

or

```bash
adb shell "cd /data/local/tmp && chmod a+x AwlTest && ./AwlTest --filter .*CompositeCompare.*"
```
