cmake_minimum_required (VERSION 2.6)
project (AwlTest)
include_directories("Awl")
add_executable(AwlTest Awl/ListTest.cpp  Awl/ObservableTest.cpp  Awl/UnitTesting.cpp  Awl/UpdateQueueTest.cpp  Awl/main.cpp)
