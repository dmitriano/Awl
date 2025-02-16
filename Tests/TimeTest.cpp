/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/String.h"
#include "Awl/Time.h"

#include "Awl/Testing/UnitTest.h"

#ifdef __STDC_LIB_EXT1__
#define __STDC_WANT_LIB_EXT1__ 1
#endif

#include <time.h>

#ifdef __STDC_LIB_EXT1__

using namespace awl::testing;
using namespace std::chrono;

using Clock = system_clock;
using TimePoint = time_point<Clock>;

template <class ...Args>
static auto MakeTime(Args... args)
{
    return awl::make_time<Clock>(args ...);
}

template <class Clock, class Duration = typename Clock::duration>
static void CheckResult(std::chrono::time_point<Clock, Duration> tp)
{
    const TimePoint stp = std::chrono::time_point_cast<typename Clock::duration>(tp);
    time_t t = Clock::to_time_t(stp);
    tm utm;
    //gmtime_s(&utm, &t);
    localtime_s(&utm, &t);
    AWL_ASSERT(utm.tm_year + 1900 == 2019);
    AWL_ASSERT(utm.tm_mon == 7);
    AWL_ASSERT(utm.tm_mday == 17);
    AWL_ASSERT(utm.tm_hour == 16);
    AWL_ASSERT(utm.tm_min == 31);
    AWL_ASSERT(utm.tm_sec == 13);
}

AWL_TEST(MakeTime)
{
    AWL_UNUSED_CONTEXT;

    constexpr std::size_t ns_count = 100200300u;
    
    auto tp_with_ns = MakeTime(2019, 07, 17, 16, 31, 13, nanoseconds(ns_count));
    CheckResult(tp_with_ns);

    TimePoint tp = MakeTime(2019, 07, 17, 16, 31, 13);
    CheckResult(tp);

    auto d = duration_cast<Clock::duration>(nanoseconds(ns_count));
    AWL_ASSERT(tp_with_ns - tp == d);

    auto d_ns = tp_with_ns - time_point_cast<nanoseconds>(tp);
    AWL_ASSERT(d_ns == nanoseconds(ns_count));
}

#endif
