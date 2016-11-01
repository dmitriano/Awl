#include <exception>
#include <iostream>

#include "UnitTesting.h"

void TestUpdateQueue();

void TestObservable();

void TestList();

void main()
{
	try
	{
		TestUpdateQueue();

		TestObservable();

		TestList();

		std::cout << std::endl << "***************** Tests passed *****************" << std::endl;
	}
	catch (const std::exception & e)
	{
		std::cout << std::endl << "***************** Tests failed: " << e.what() << std::endl;
	}
}