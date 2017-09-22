#include <iostream>
#include <algorithm>

#include "Awl/ScopeGuard.h"

#include "UnitTesting.h"

using namespace UnitTesting;

void ScopeGuardTest()
{
	int usage = 1;

	{
		auto guard = awl::make_scope_guard([&usage]() { --usage; });

		guard.release();

		Assert::AreEqual(usage, 1);
	}

	Assert::AreEqual(usage, 1);

	{
		auto guard = awl::make_scope_guard([&usage]() { --usage; });

		Assert::AreEqual(usage, 1);
	}

	Assert::AreEqual(usage, 0);
}
