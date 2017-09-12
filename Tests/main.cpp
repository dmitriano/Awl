#include <exception>
#include <iostream>

#include "UnitTesting.h"

void TestUpdateQueue();

void TestObservable();

void TestList();

int main()
{
    int error = 1;

    try
	{
		TestList();

		TestUpdateQueue();

		TestObservable();

		std::cout << std::endl << "***************** Tests passed *****************" << std::endl;

        error = 0;
	}
	catch (const std::exception & e)
	{
		std::cout << std::endl << "***************** Tests failed: " << e.what() << std::endl;
	}

    return error;
}
