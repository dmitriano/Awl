#include "Awl/Coro/SignalAwaitable.h"
#include "Awl/Coro/Job.h"
#include "Awl/Testing/UnitTest.h"

#include <memory>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

namespace
{
    class RangeSender
    {
    public:

        explicit RangeSender(int init_id) :
            id(init_id)
        {}

        int id;
        bool enabled = true;
        awl::Signal<int> changed;
        awl::Signal<> closed;

        awl::Signal<int>& changedSignal()
        {
            return changed;
        }
    };

    awl::Job waitInt(awl::Signal<int>& signal, int& result)
    {
        result = co_await awl::wait_signal(signal);
    }

    awl::Job waitTuple(
        awl::Signal<const std::shared_ptr<int>&, const std::string&>& signal,
        std::shared_ptr<int>& sender,
        std::string& value)
    {
        auto [result_sender, result_value] = co_await awl::wait_signal(signal);

        sender = std::move(result_sender);
        value = std::move(result_value);
    }

    awl::Job waitVoid(awl::Signal<>& signal, bool& resumed)
    {
        co_await awl::wait_signal(signal);

        resumed = true;
    }

    template <class R>
    awl::Job waitRangeValue(R&& objects, std::shared_ptr<RangeSender>& sender, int& value)
    {
        auto [result_sender, result_value] = co_await awl::wait_signal<&RangeSender::changed>(std::forward<R>(objects));

        sender = std::move(result_sender);
        value = result_value;
    }

    template <class R>
    awl::Job waitRangeMethodValue(R&& objects, std::shared_ptr<RangeSender>& sender, int& value)
    {
        auto [result_sender, result_value] = co_await awl::wait_signal<&RangeSender::changedSignal>(std::forward<R>(objects));

        sender = std::move(result_sender);
        value = result_value;
    }

    template <class R>
    awl::Job waitRangeVoid(R&& objects, std::shared_ptr<RangeSender>& sender)
    {
        sender = co_await awl::wait_signal<&RangeSender::closed>(std::forward<R>(objects));
    }
}

AWL_TEST(SignalAwaitable_SingleArgument)
{
    AWL_UNUSED_CONTEXT;

    awl::Signal<int> signal;
    int result = 0;

    awl::Job job = waitInt(signal, result);

    AWL_ASSERT(!job.done());
    AWL_ASSERT_EQUAL(1u, signal.size());

    signal.emit(7);

    AWL_ASSERT(job.done());
    AWL_ASSERT_EQUAL(7, result);
    AWL_ASSERT(signal.empty());
}

AWL_TEST(SignalAwaitable_Tuple)
{
    AWL_UNUSED_CONTEXT;

    awl::Signal<const std::shared_ptr<int>&, const std::string&> signal;
    auto expected_sender = std::make_shared<int>(42);
    std::shared_ptr<int> sender;
    std::string value;

    awl::Job job = waitTuple(signal, sender, value);

    AWL_ASSERT(!job.done());

    signal.emit(expected_sender, "done");

    AWL_ASSERT(job.done());
    AWL_ASSERT(sender == expected_sender);
    AWL_ASSERT_EQUAL(std::string("done"), value);
    AWL_ASSERT(signal.empty());
}

AWL_TEST(SignalAwaitable_Void)
{
    AWL_UNUSED_CONTEXT;

    awl::Signal<> signal;
    bool resumed = false;

    awl::Job job = waitVoid(signal, resumed);

    AWL_ASSERT(!job.done());

    signal.emit();

    AWL_ASSERT(job.done());
    AWL_ASSERT(resumed);
    AWL_ASSERT(signal.empty());
}

AWL_TEST(SignalAwaitable_ResumesOnce)
{
    AWL_UNUSED_CONTEXT;

    awl::Signal<int> signal;
    int result = 0;

    awl::Job job = waitInt(signal, result);

    signal.emit(1);
    signal.emit(2);

    AWL_ASSERT(job.done());
    AWL_ASSERT_EQUAL(1, result);
    AWL_ASSERT(signal.empty());
}

AWL_TEST(SignalAwaitable_UnsubscribesOnCancel)
{
    AWL_UNUSED_CONTEXT;

    awl::Signal<int> signal;
    int result = 0;

    {
        awl::Job job = waitInt(signal, result);

        AWL_ASSERT(!job.done());
        AWL_ASSERT_EQUAL(1u, signal.size());
    }

    AWL_ASSERT(signal.empty());

    signal.emit(3);

    AWL_ASSERT_EQUAL(0, result);
}

AWL_TEST(SignalRangeAwaitable_Value)
{
    AWL_UNUSED_CONTEXT;

    std::vector<std::shared_ptr<RangeSender>> objects =
    {
        std::make_shared<RangeSender>(1),
        std::make_shared<RangeSender>(2),
        std::make_shared<RangeSender>(3)
    };

    std::shared_ptr<RangeSender> sender;
    int value = 0;

    awl::Job job = waitRangeValue(objects, sender, value);

    AWL_ASSERT(!job.done());

    for (const std::shared_ptr<RangeSender>& object : objects)
    {
        AWL_ASSERT_EQUAL(1u, object->changed.size());
    }

    objects[1]->changed.emit(17);

    AWL_ASSERT(job.done());
    AWL_ASSERT(sender == objects[1]);
    AWL_ASSERT_EQUAL(2, sender->id);
    AWL_ASSERT_EQUAL(17, value);

    for (const std::shared_ptr<RangeSender>& object : objects)
    {
        AWL_ASSERT(object->changed.empty());
    }

    objects[2]->changed.emit(23);

    AWL_ASSERT_EQUAL(17, value);
}

AWL_TEST(SignalRangeAwaitable_MemberFunction)
{
    AWL_UNUSED_CONTEXT;

    std::vector<std::shared_ptr<RangeSender>> objects =
    {
        std::make_shared<RangeSender>(1),
        std::make_shared<RangeSender>(2)
    };

    std::shared_ptr<RangeSender> sender;
    int value = 0;

    awl::Job job = waitRangeMethodValue(objects, sender, value);

    AWL_ASSERT(!job.done());
    AWL_ASSERT_EQUAL(1u, objects[0]->changed.size());
    AWL_ASSERT_EQUAL(1u, objects[1]->changed.size());

    objects[1]->changed.emit(31);

    AWL_ASSERT(job.done());
    AWL_ASSERT(sender == objects[1]);
    AWL_ASSERT_EQUAL(31, value);
    AWL_ASSERT(objects[0]->changed.empty());
    AWL_ASSERT(objects[1]->changed.empty());
}

AWL_TEST(SignalRangeAwaitable_Void)
{
    AWL_UNUSED_CONTEXT;

    std::vector<std::shared_ptr<RangeSender>> objects =
    {
        std::make_shared<RangeSender>(1),
        std::make_shared<RangeSender>(2)
    };

    std::shared_ptr<RangeSender> sender;

    awl::Job job = waitRangeVoid(objects, sender);

    AWL_ASSERT(!job.done());
    AWL_ASSERT_EQUAL(1u, objects[0]->closed.size());
    AWL_ASSERT_EQUAL(1u, objects[1]->closed.size());

    objects[0]->closed.emit();

    AWL_ASSERT(job.done());
    AWL_ASSERT(sender == objects[0]);
    AWL_ASSERT_EQUAL(1, sender->id);
    AWL_ASSERT(objects[0]->closed.empty());
    AWL_ASSERT(objects[1]->closed.empty());
}

AWL_TEST(SignalRangeAwaitable_FilterView)
{
    AWL_UNUSED_CONTEXT;

    std::vector<std::shared_ptr<RangeSender>> objects =
    {
        std::make_shared<RangeSender>(1),
        std::make_shared<RangeSender>(2),
        std::make_shared<RangeSender>(3)
    };

    objects[0]->enabled = false;
    objects[2]->enabled = false;

    auto enabled_objects = objects | std::views::filter([](const std::shared_ptr<RangeSender>& object)
    {
        return object->enabled;
    });

    std::shared_ptr<RangeSender> sender;
    int value = 0;

    awl::Job job = waitRangeValue(enabled_objects, sender, value);

    AWL_ASSERT(!job.done());
    AWL_ASSERT(objects[0]->changed.empty());
    AWL_ASSERT_EQUAL(1u, objects[1]->changed.size());
    AWL_ASSERT(objects[2]->changed.empty());

    objects[0]->changed.emit(11);

    AWL_ASSERT(!job.done());
    AWL_ASSERT(sender == nullptr);
    AWL_ASSERT_EQUAL(0, value);

    objects[1]->changed.emit(29);

    AWL_ASSERT(job.done());
    AWL_ASSERT(sender == objects[1]);
    AWL_ASSERT_EQUAL(29, value);
    AWL_ASSERT(objects[1]->changed.empty());
}

AWL_TEST(SignalRangeAwaitable_UnsubscribesOnCancel)
{
    AWL_UNUSED_CONTEXT;

    std::weak_ptr<RangeSender> weak1;
    std::weak_ptr<RangeSender> weak2;
    std::shared_ptr<RangeSender> sender;
    int value = 0;

    {
        std::vector<std::shared_ptr<RangeSender>> objects =
        {
            std::make_shared<RangeSender>(1),
            std::make_shared<RangeSender>(2)
        };

        weak1 = objects[0];
        weak2 = objects[1];

        {
            awl::Job job = waitRangeValue(objects, sender, value);

            AWL_ASSERT(!job.done());
            AWL_ASSERT_EQUAL(1u, objects[0]->changed.size());
            AWL_ASSERT_EQUAL(1u, objects[1]->changed.size());
        }

        AWL_ASSERT(objects[0]->changed.empty());
        AWL_ASSERT(objects[1]->changed.empty());

        objects.clear();
    }

    AWL_ASSERT(weak1.expired());
    AWL_ASSERT(weak2.expired());
    AWL_ASSERT(sender == nullptr);
    AWL_ASSERT_EQUAL(0, value);
}
