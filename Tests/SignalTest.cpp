/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Signal.h"
#include "Awl/Testing/UnitTest.h"

namespace
{
    class Handler
    {
    public:

        void on_value(int value)
        {
            sum += value;
            ++count;
        }

        int sum = 0;
        int count = 0;
    };
}

AWL_TEST(Signal_SubscribeUnsubscribeEmit)
{
    AWL_UNUSED_CONTEXT;

    awl::Signal<int> signal;
    Handler h1;
    Handler h2;

    AWL_ASSERT(signal.empty());
    AWL_ASSERT_EQUAL(0u, signal.size());

    signal.subscribe(&h1, &Handler::on_value);
    signal.subscribe(&h2, &Handler::on_value);
    signal.subscribe(&h1, &Handler::on_value);

    AWL_ASSERT_EQUAL(2u, signal.size());

    signal.emit(3);

    AWL_ASSERT_EQUAL(3, h1.sum);
    AWL_ASSERT_EQUAL(3, h2.sum);
    AWL_ASSERT_EQUAL(1, h1.count);
    AWL_ASSERT_EQUAL(1, h2.count);

    AWL_ASSERT(signal.unsubscribe(&h1, &Handler::on_value));
    AWL_ASSERT_FALSE(signal.unsubscribe(&h1, &Handler::on_value));
    AWL_ASSERT_EQUAL(1u, signal.size());

    signal.emit(4);

    AWL_ASSERT_EQUAL(3, h1.sum);
    AWL_ASSERT_EQUAL(7, h2.sum);
    AWL_ASSERT_EQUAL(1, h1.count);
    AWL_ASSERT_EQUAL(2, h2.count);

    signal.clear();
    AWL_ASSERT(signal.empty());
    AWL_ASSERT_EQUAL(0u, signal.size());
}
