#include "UnitTesting.h"

namespace UnitTesting
{
	const char * TestException::what() const throw()
	{
		return theMessage.c_str();
	}
}
