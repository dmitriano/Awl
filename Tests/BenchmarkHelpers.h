#pragma once

#include "Awl/StopWatch.h"
#include "Awl/Testing/UnitTest.h"

double ReportSpeed(const awl::testing::TestContext & context, const awl::StopWatch & w, size_t size);

double ReportCount(const awl::testing::TestContext & context, const awl::StopWatch & w, size_t count);
