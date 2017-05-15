cmake_minimum_required (VERSION 2.8.12)
project (AwlTest)
include_directories("Awl")

#http://stackoverflow.com/questions/37621342/cmake-will-not-compile-to-c-11-standard
add_compile_options(-std=c++11) # CMake 2.8.12 or newer
#In CMake version 3.1 and newer, if you want to force C++11, you need the following lines:
#set(CMAKE_CXX_STANDARD 11) # C++11...
#set(CMAKE_CXX_STANDARD_REQUIRED ON) #...is required...
#set(CMAKE_CXX_EXTENSIONS OFF) #...without compiler extensions like gnu++11

add_executable(AwlTest Awl/ListTest.cpp  Awl/ObservableTest.cpp  Awl/UnitTesting.cpp  Awl/UpdateQueueTest.cpp  Awl/main.cpp)
