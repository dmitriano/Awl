# Awl
AWL - A Working Library

AWL is a small cross-platform C++ library that includes:

1. A simple [binary serialization framework](https://developernote.com/2024/07/version-tolerant-serialization-in-c/) and simple [JSON serialization framework](https://github.com/dmitriano/Awl/tree/master/Extras/Qt/QtExtras/Json).
2. Memory stream, buffered stream, hashing stream.
3. A [set](https://github.com/dmitriano/Awl/blob/master/Awl/VectorSet.h) that finds an element by both key and index with O(logN) time.
4. A [doubly linked list](https://github.com/dmitriano/Awl/blob/master/Awl/QuickList.h) with static `insert` and `erase` methods and movable elements.
5. A movable [observable](https://github.com/dmitriano/Awl/blob/master/Awl/Observable.h) with movable observers.
6. [Bitset based on enum](https://github.com/dmitriano/Awl/blob/master/Awl/BitMap.h).
7. A [circular buffer](https://github.com/dmitriano/Awl/blob/master/Awl/Ring.h) with an interface similar to std::queue.
8. Other simple classes like CompositeCompare, ReverseCompare, scope_guard, etc...
9. A simple [testing framework](https://github.com/dmitriano/Awl/tree/master/Awl/Testing).

Theoretically, the master branch should compile with C++20 and work, at least it is periodically built with `MSVC 19.44.35219`, `GCC 13.3.0`, `Android CLang 20.0 (from NDK 29.0.13113456)` and `Apple Clang 1700.0.13.5 (on MacOS Sonoma with Xcode 16.4)`.

There is also cpp17 branch that partially compiles with C++17.

Version compatibility is not guaranteed and there is no warranty of any kind.

Feel free to use it or fork it, report a bug by opening an issue.

To leave the author a message fill the [form on his website](https://developernote.com/contact/).

## Compiling on Windows with MSVC 2022:

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

## Compiling on Linux with Ninja generator:

```bash
cmake ../../repos/Awl/ -G Ninja
cmake --build . --parallel --target AwlTest --config RelWithDebInfo
```

Compiling a single source file (by example of `VtsTest.cpp`):

```bash
cmake --build . --parallel --target CMakeFiles/AwlTest.dir/Tests/VtsTest.cpp.o
```

## Compiling on Linux without Ninja generator:

```bash
cmake ../../Awl/ -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel
```

or

```bash
cmake ../../Awl/ -DCMAKE_BUILD_TYPE=Debug
cmake --build . --parallel
```

## Compiling with OpenSSL, QT and Boost on Windows

Use `-DAWL_STATIC_RUNTIME:BOOL=ON` CMake option if QT and BOOST are compiled with static runtime:

```bat
set MY_DRIVE=C:

%MY_DRIVE%
cd \dev\build\awl

set MY_CMAKE_EXE=%MY_DRIVE%\dev\tools\cmake-4.1.2-windows-x86_64\bin\cmake.exe
set MY_QT_DIR=%MY_DRIVE%\dev\libs\Qt6\windows
set MY_BOOST_DIR=%MY_DRIVE%\dev\libs\boost_1_89_0
set MY_VS_GENERATOR="Visual Studio 17 2022"

set OPENSSL_ROOT_DIR=%MY_DRIVE%/dev/libs/OpenSSL
set OPENSSL_USE_STATIC_LIBS=ON

%MY_CMAKE_EXE% ..\..\repos\Awl -G %MY_VS_GENERATOR% -A x64 -DCMAKE_PREFIX_PATH="%MY_QT_DIR%;%MY_BOOST_DIR%" -DAWL_FIND_OPENSSL:BOOL=ON -DAWL_FIND_BOOST:BOOL=ON -DAWL_FIND_QT:BOOL=ON -DAWL_STATIC_RUNTIME:BOOL=ON -DAWL_ANSI_CMD_CHAR:BOOL=ON
%MY_CMAKE_EXE% --build . --parallel --target AwlTest --config Debug
%MY_CMAKE_EXE% --build . --parallel --target AwlTest --config RelWithDebInfo
```

## Compiling on Ubuntu 24.04 with OpenSSL 3.5.4, BOOST 1.89.0

```bash
export OPENSSL_ROOT_DIR=/home/dmitriano/dev/libs/OpenSSL
export OPENSSL_USE_STATIC_LIBS=ON

cmake ../../repos/Awl/ -G Ninja -DCMAKE_PREFIX_PATH="/home/dmitriano/dev/libs/boost" -DAWL_FIND_OPENSSL:BOOL=ON -DAWL_FIND_BOOST:BOOL=ON -DAWL_STATIC_RUNTIME:BOOL=ON
cmake --build . --parallel
```

## Using GCC sanitizer

Add `-DAWL_SANITIZE_ADDRESS=1` or `-DAWL_SANITIZE_UNDEFINED=1` or `-DAWL_SANITIZE_THREAD=1` to CMake command, for example:

```bash
cmake ../../repos/Awl/ -G Ninja -DAWL_SANITIZE_ADDRESS=1
cmake --build . --parallel --target AwlTest --config RelWithDebInfo
```

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

Running unstable tests and examples:

```bash
./AwlTest --filter ".*_Unstable"
```

or

```bash
./AwlTest --filter ".*"
```

## Running the tests on Android device

Built AWL for Android with `-DAWL_STATIC_RUNTIME:BOOL=ON` CMake option and if you use QT Creator do not forget to set `-DAWL_FIND_QT:BOOL=OFF`, upload the executable file to the device:

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
