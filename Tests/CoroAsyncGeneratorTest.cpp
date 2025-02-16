/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Separator.h"
#include "Awl/Coro/UpdateTask.h"
#include "Awl/Coro/ProcessTask.h"
#include "Awl/Coro/AsyncGenerator.h"

#include "Awl/Testing/UnitTest.h"
#include "Awl/Testing/TimeQueue.h"

//Why does it fail?
//static_assert(std::ranges::range<awl::async_generator<int>>);

namespace
{
    using awl::testing::operator co_await;
    using namespace std::chrono_literals;

    awl::async_generator<int> gen(int count)
    {
        for (int i = 0; i < count; ++i)
        {
            co_await 100ms;

            if (i > 5)
            {
                throw std::runtime_error("Generator overflow.");
            }

            co_yield i;
        }
    }

    awl::ProcessTask<void> print(const awl::testing::TestContext& context, int count, std::optional<int> limit = {})
    {
        awl::separator sep(_T(','));
        
        //Unfortunately, 'for co_await' syntax is not approved for C++20 (I hope for now!) and instead of an elegant code we have to write
        //old school for loop with previously captured by rvalue generator.
        //for co_await(int i : gen())

        auto g = gen(count);

        int n = 0;

        for (auto i = co_await g.begin(); i != g.end(); co_await ++i)
        {
            context.out << sep << *i;
            context.out.flush();

            if (limit && ++n == *limit)
            {
                break;
            }
        }

        context.out << std::endl;
    }

    awl::UpdateTask test(const awl::testing::TestContext& context)
    {
        co_await print(context, 3);

        co_await print(context, 10, 2);

        try
        {
            co_await print(context, 10);

            AWL_FAILM(_T("AsyncGenerator did not throw."));
        }
        catch (const std::exception& ex)
        {
            context.out << std::endl << "Exception: " << ex.what() << std::endl;
        }
        catch (...)
        {
            AWL_FAILM(_T("AsyncGenerator thrown a wrong exception."));
        }
    }
}

AWL_TEST(CoroAsyncGenerator)
{
    awl::UpdateTask task = test(context);

    awl::testing::timeQueue.loop();
}
