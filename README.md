# Awl
Awl - A working library

Awl is a small cross-platform C++ library used in various real-life projects including [Line 3D game](http://doc.developernote.com/linesgame/), it is compiled with MSVC, Clang and GCC.

There is no documentation on Awl yet, but the blog post [Synchronization mechanism in DirectX 11 and XAML UWP application](http://developernote.com/2015/11/synchronization-mechanism-in-directx-11-and-xaml-winrt-application/) illustrates the idea awl::UpdateQueue class based on by example of some Microsoft-specific C++/CX code.

Build Instruction:

On Linux:
Create 'build' directory in the repository root directory.
execute commands:
cmake ../ -DCMAKE_BUILD_TYPE=debug
make

On Windows:
Open Awl.sln in VS2015 and press F5.
