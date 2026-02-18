/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Signal.h"
#include "Awl/Testing/UnitTest.h"

#include <functional>
#include <memory>

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

AWL_TEST(Signal_StdFunction)
{
    AWL_UNUSED_CONTEXT;

    awl::Signal<int> signal;
    int sum1 = 0;
    int sum2 = 0;

    const auto id1 = signal.subscribe(std::function<void(int)>([&sum1](int value)
    {
        sum1 += value;
    }));

    const auto id2 = signal.subscribe(std::function<void(int)>([&sum2](int value)
    {
        sum2 += value * 2;
    }));

    AWL_ASSERT_FALSE(id1 == id2);
    AWL_ASSERT_EQUAL(2u, signal.size());

    signal.emit(3);
    AWL_ASSERT_EQUAL(3, sum1);
    AWL_ASSERT_EQUAL(6, sum2);

    AWL_ASSERT(signal.unsubscribe(id1));
    AWL_ASSERT_FALSE(signal.unsubscribe(id1));
    AWL_ASSERT_EQUAL(1u, signal.size());

    signal.emit(4);
    AWL_ASSERT_EQUAL(3, sum1);
    AWL_ASSERT_EQUAL(14, sum2);

    AWL_ASSERT(signal.unsubscribe(id2));
    AWL_ASSERT(signal.empty());
}

AWL_TEST(Signal_WeakPtr)
{
    AWL_UNUSED_CONTEXT;

    awl::Signal<int> signal;

    auto owner = std::make_shared<Handler>();
    std::weak_ptr<Handler> weak = owner;

    signal.subscribe(weak, &Handler::on_value);
    signal.subscribe(weak, &Handler::on_value);

    // Same target must be deduplicated.
    AWL_ASSERT_EQUAL(1u, signal.size());

    signal.emit(5);
    AWL_ASSERT_EQUAL(5, owner->sum);
    AWL_ASSERT_EQUAL(1, owner->count);

    owner.reset();
    AWL_ASSERT(weak.expired());

    signal.emit(7);
    AWL_ASSERT(signal.empty());
    AWL_ASSERT_FALSE(signal.unsubscribe(weak, &Handler::on_value));
}

AWL_TEST(Signal_WeakPtrCompaction)
{
    AWL_UNUSED_CONTEXT;

    awl::Signal<int> signal;

    auto owner_alive = std::make_shared<Handler>();
    auto owner_dead = std::make_shared<Handler>();
    std::weak_ptr<Handler> weak_alive = owner_alive;
    std::weak_ptr<Handler> weak_dead = owner_dead;

    signal.subscribe(weak_alive, &Handler::on_value);
    signal.subscribe(weak_dead, &Handler::on_value);

    owner_dead.reset();
    AWL_ASSERT(weak_dead.expired());

    signal.emit(10);

    AWL_ASSERT_EQUAL(1u, signal.size());
    AWL_ASSERT_EQUAL(10, owner_alive->sum);
    AWL_ASSERT_EQUAL(1, owner_alive->count);
}

AWL_TEST(Signal_RemoveExpiredSlotInEmit)
{
    AWL_UNUSED_CONTEXT;

    awl::Signal<int> signal;

    auto owner1 = std::make_shared<Handler>();
    auto owner_dead = std::make_shared<Handler>();
    auto owner2 = std::make_shared<Handler>();
    std::weak_ptr<Handler> weak1 = owner1;
    std::weak_ptr<Handler> weak_dead = owner_dead;
    std::weak_ptr<Handler> weak2 = owner2;

    signal.subscribe(weak1, &Handler::on_value);
    signal.subscribe(weak_dead, &Handler::on_value);
    signal.subscribe(weak2, &Handler::on_value);

    AWL_ASSERT_EQUAL(3u, signal.size());

    owner_dead.reset();
    AWL_ASSERT(weak_dead.expired());

    signal.emit(2);

    AWL_ASSERT_EQUAL(2u, signal.size());
    AWL_ASSERT_EQUAL(2, owner1->sum);
    AWL_ASSERT_EQUAL(1, owner1->count);
    AWL_ASSERT_EQUAL(2, owner2->sum);
    AWL_ASSERT_EQUAL(1, owner2->count);
    AWL_ASSERT_FALSE(signal.unsubscribe(weak_dead, &Handler::on_value));

    signal.emit(3);

    AWL_ASSERT_EQUAL(2u, signal.size());
    AWL_ASSERT_EQUAL(5, owner1->sum);
    AWL_ASSERT_EQUAL(2, owner1->count);
    AWL_ASSERT_EQUAL(5, owner2->sum);
    AWL_ASSERT_EQUAL(2, owner2->count);
}
