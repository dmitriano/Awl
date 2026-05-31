#include "Awl/Coro/SignalAccumulator.h"
#include "Awl/Coro/SignalAwaitable.h"
#include "Awl/Coro/Job.h"
#include "Awl/Coro/SignalRangeAccumulator.h"
#include "Awl/ObservableSet.h"
#include "Awl/ObservableUnorderedSet.h"
#include "Awl/Source.h"
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
        awl::Source<int> changed;
        awl::Source<> closed;

        decltype(changed)::Signal& changedSignal()
        {
            return changed.signal();
        }

        decltype(closed)::Signal& closedSignal()
        {
            return closed.signal();
        }
    };

    awl::coro::Job waitInt(awl::ISignal<int>& signal, int& result)
    {
        result = co_await awl::coro::wait_signal(signal);
    }

    awl::coro::Job waitMovedInt(awl::ISignal<int>& signal, int& result)
    {
        awl::coro::SignalAwaitable<int> awaitable = awl::coro::wait_signal(signal);
        awl::coro::SignalAwaitable<int> moved(std::move(awaitable));

        result = co_await moved;
    }

    awl::coro::Job waitTuple(
        awl::ISignal<const std::shared_ptr<int>&, const std::string&>& signal,
        std::shared_ptr<int>& sender,
        std::string& value)
    {
        auto [result_sender, result_value] = co_await awl::coro::wait_signal(signal);

        sender = std::move(result_sender);
        value = std::move(result_value);
    }

    awl::coro::Job waitVoid(awl::ISignal<>& signal, bool& resumed)
    {
        co_await awl::coro::wait_signal(signal);

        resumed = true;
    }

    awl::coro::Job waitAccumulatedInt(awl::coro::SignalAccumulator<int>& accumulator, int& result)
    {
        result = co_await accumulator.wait();
    }

    awl::coro::Job waitAccumulatedInts(awl::coro::SignalAccumulator<int>& accumulator, std::vector<int>& values)
    {
        for (int i = 0; i != 2; ++i)
        {
            values.push_back(co_await accumulator.wait());
        }
    }

    awl::coro::Job waitAccumulatedVoid(awl::coro::SignalAccumulator<>& accumulator, int& count)
    {
        co_await accumulator.wait();
        ++count;

        co_await accumulator.wait();
        ++count;
    }

    awl::coro::Job emitDuringAccumulatedProcessing(
        awl::coro::SignalAccumulator<int>& accumulator,
        awl::Source<int>& signal,
        std::vector<int>& values)
    {
        values.push_back(co_await accumulator.wait());

        signal.emit(2);

        values.push_back(co_await accumulator.wait());
    }

    template <class R>
    awl::coro::Job waitRangeValue(R&& objects, std::shared_ptr<RangeSender>& sender, int& value)
    {
        {
            auto accumulator = awl::coro::accumulate_signal<&RangeSender::changedSignal>(std::forward<R>(objects));
            auto [result_sender, result_value] = co_await accumulator.wait();

            sender = std::move(result_sender);
            value = result_value;
        }
    }

    template <class R>
    awl::coro::Job waitRangeMethodValue(R&& objects, std::shared_ptr<RangeSender>& sender, int& value)
    {
        {
            auto accumulator = awl::coro::accumulate_signal<&RangeSender::changedSignal>(std::forward<R>(objects));
            auto [result_sender, result_value] = co_await accumulator.wait();

            sender = std::move(result_sender);
            value = result_value;
        }
    }

    template <class R>
    awl::coro::Job waitRangeVoid(R&& objects, std::shared_ptr<RangeSender>& sender)
    {
        {
            auto accumulator = awl::coro::accumulate_signal<&RangeSender::closedSignal>(std::forward<R>(objects));

            sender = co_await accumulator.wait();
        }
    }

    using RangeChangedAccumulator = awl::coro::SignalRangeAccumulator<RangeSender, &RangeSender::changedSignal>;

    awl::coro::Job waitAccumulatedValue(RangeChangedAccumulator& accumulator, std::shared_ptr<RangeSender>& sender, int& value)
    {
        auto [result_sender, result_value] = co_await accumulator.wait();

        sender = std::move(result_sender);
        value = result_value;
    }

    awl::coro::Job waitAccumulatedValues(
        RangeChangedAccumulator& accumulator,
        std::vector<std::shared_ptr<RangeSender>>& senders,
        std::vector<int>& values)
    {
        for (int i = 0; i != 2; ++i)
        {
            auto [sender, value] = co_await accumulator.wait();

            senders.push_back(std::move(sender));
            values.push_back(value);
        }
    }
}

AWL_TEST(SignalAwaitable_SingleArgument)
{
    AWL_UNUSED_CONTEXT;

    awl::Source<int> signal;
    int result = 0;

    awl::coro::Job job = waitInt(signal.signal(), result);

    AWL_ASSERT(!job.done());
    AWL_ASSERT_EQUAL(1u, signal.size());

    signal.emit(7);

    AWL_ASSERT(job.done());
    AWL_ASSERT_EQUAL(7, result);
    AWL_ASSERT(signal.empty());
}

AWL_TEST(SignalAwaitable_MoveConstructedBeforeAwait)
{
    AWL_UNUSED_CONTEXT;

    awl::Source<int> signal;
    int result = 0;

    awl::coro::Job job = waitMovedInt(signal.signal(), result);

    AWL_ASSERT(!job.done());
    AWL_ASSERT_EQUAL(1u, signal.size());

    signal.emit(9);

    AWL_ASSERT(job.done());
    AWL_ASSERT_EQUAL(9, result);
    AWL_ASSERT(signal.empty());
}

AWL_TEST(SignalAwaitable_Tuple)
{
    AWL_UNUSED_CONTEXT;

    awl::Source<const std::shared_ptr<int>&, const std::string&> signal;
    std::shared_ptr<int> expected_sender = std::make_shared<int>(42);
    std::shared_ptr<int> sender;
    std::string value;

    awl::coro::Job job = waitTuple(signal.signal(), sender, value);

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

    awl::Source<> signal;
    bool resumed = false;

    awl::coro::Job job = waitVoid(signal.signal(), resumed);

    AWL_ASSERT(!job.done());

    signal.emit();

    AWL_ASSERT(job.done());
    AWL_ASSERT(resumed);
    AWL_ASSERT(signal.empty());
}

AWL_TEST(SignalAwaitable_ResumesOnce)
{
    AWL_UNUSED_CONTEXT;

    awl::Source<int> signal;
    int result = 0;

    awl::coro::Job job = waitInt(signal.signal(), result);

    signal.emit(1);
    signal.emit(2);

    AWL_ASSERT(job.done());
    AWL_ASSERT_EQUAL(1, result);
    AWL_ASSERT(signal.empty());
}

AWL_TEST(SignalAwaitable_UnsubscribesOnCancel)
{
    AWL_UNUSED_CONTEXT;

    awl::Source<int> signal;
    int result = 0;

    {
        awl::coro::Job job = waitInt(signal.signal(), result);

        AWL_ASSERT(!job.done());
        AWL_ASSERT_EQUAL(1u, signal.size());
    }

    AWL_ASSERT(signal.empty());

    signal.emit(3);

    AWL_ASSERT_EQUAL(0, result);
}

AWL_TEST(SignalAccumulator_QueuesBeforeWait)
{
    AWL_UNUSED_CONTEXT;

    awl::Source<int> signal;
    awl::coro::SignalAccumulator<int> accumulator(signal.signal());

    signal.emit(11);

    int result = 0;
    awl::coro::Job job = waitAccumulatedInt(accumulator, result);

    AWL_ASSERT(job.done());
    AWL_ASSERT_EQUAL(11, result);
    AWL_ASSERT_EQUAL(1u, signal.size());
}

AWL_TEST(SignalAccumulator_QueuesInOrder)
{
    AWL_UNUSED_CONTEXT;

    awl::Source<int> signal;
    awl::coro::SignalAccumulator<int> accumulator(signal.signal());

    signal.emit(13);
    signal.emit(17);

    std::vector<int> values;
    awl::coro::Job job = waitAccumulatedInts(accumulator, values);

    AWL_ASSERT(job.done());
    AWL_ASSERT_EQUAL(2u, values.size());
    AWL_ASSERT_EQUAL(13, values[0]);
    AWL_ASSERT_EQUAL(17, values[1]);
}

AWL_TEST(SignalAccumulator_WaitMany)
{
    AWL_UNUSED_CONTEXT;

    awl::Source<int> signal;
    awl::coro::SignalAccumulator<int> accumulator(signal.signal());
    std::vector<int> values;

    awl::coro::Job job = waitAccumulatedInts(accumulator, values);

    AWL_ASSERT(!job.done());

    signal.emit(19);

    AWL_ASSERT(!job.done());
    AWL_ASSERT_EQUAL(1u, values.size());
    AWL_ASSERT_EQUAL(19, values[0]);

    signal.emit(23);

    AWL_ASSERT(job.done());
    AWL_ASSERT_EQUAL(2u, values.size());
    AWL_ASSERT_EQUAL(23, values[1]);
}

AWL_TEST(SignalAccumulator_QueuesDuringProcessing)
{
    AWL_UNUSED_CONTEXT;

    awl::Source<int> signal;
    awl::coro::SignalAccumulator<int> accumulator(signal.signal());
    std::vector<int> values;

    awl::coro::Job job = emitDuringAccumulatedProcessing(accumulator, signal, values);

    AWL_ASSERT(!job.done());

    signal.emit(29);

    AWL_ASSERT(job.done());
    AWL_ASSERT_EQUAL(2u, values.size());
    AWL_ASSERT_EQUAL(29, values[0]);
    AWL_ASSERT_EQUAL(2, values[1]);
}

AWL_TEST(SignalAccumulator_Void)
{
    AWL_UNUSED_CONTEXT;

    awl::Source<> signal;
    awl::coro::SignalAccumulator<> accumulator(signal.signal());
    int count = 0;

    signal.emit();
    signal.emit();

    awl::coro::Job job = waitAccumulatedVoid(accumulator, count);

    AWL_ASSERT(job.done());
    AWL_ASSERT_EQUAL(2, count);
}

AWL_TEST(SignalAccumulator_Unsubscribes)
{
    AWL_UNUSED_CONTEXT;

    awl::Source<int> signal;

    {
        awl::coro::SignalAccumulator<int> accumulator(signal.signal());

        AWL_ASSERT_EQUAL(1u, signal.size());
    }

    AWL_ASSERT(signal.empty());
}

AWL_TEST(SignalRangeAccumulator_Value)
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

    awl::coro::Job job = waitRangeValue(objects, sender, value);

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

AWL_TEST(SignalRangeAccumulator_MemberFunction)
{
    AWL_UNUSED_CONTEXT;

    std::vector<std::shared_ptr<RangeSender>> objects =
    {
        std::make_shared<RangeSender>(1),
        std::make_shared<RangeSender>(2)
    };

    std::shared_ptr<RangeSender> sender;
    int value = 0;

    awl::coro::Job job = waitRangeMethodValue(objects, sender, value);

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

AWL_TEST(SignalRangeAccumulator_Void)
{
    AWL_UNUSED_CONTEXT;

    std::vector<std::shared_ptr<RangeSender>> objects =
    {
        std::make_shared<RangeSender>(1),
        std::make_shared<RangeSender>(2)
    };

    std::shared_ptr<RangeSender> sender;

    awl::coro::Job job = waitRangeVoid(objects, sender);

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

AWL_TEST(SignalRangeAccumulator_FilterView)
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

    awl::coro::Job job = waitRangeValue(enabled_objects, sender, value);

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

AWL_TEST(SignalRangeAccumulator_UnsubscribesOnCancel)
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
            awl::coro::Job job = waitRangeValue(objects, sender, value);

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

AWL_TEST(SignalRangeAccumulator_QueuesBeforeWait)
{
    AWL_UNUSED_CONTEXT;

    std::vector<std::shared_ptr<RangeSender>> objects =
    {
        std::make_shared<RangeSender>(1),
        std::make_shared<RangeSender>(2)
    };

    RangeChangedAccumulator accumulator(objects);

    AWL_ASSERT_EQUAL(1u, objects[0]->changed.size());
    AWL_ASSERT_EQUAL(1u, objects[1]->changed.size());

    objects[1]->changed.emit(61);

    std::shared_ptr<RangeSender> sender;
    int value = 0;

    awl::coro::Job job = waitAccumulatedValue(accumulator, sender, value);

    AWL_ASSERT(job.done());
    AWL_ASSERT(sender == objects[1]);
    AWL_ASSERT_EQUAL(61, value);
    AWL_ASSERT_EQUAL(1u, objects[0]->changed.size());
    AWL_ASSERT_EQUAL(1u, objects[1]->changed.size());
}

AWL_TEST(SignalRangeAccumulator_WaitMany)
{
    AWL_UNUSED_CONTEXT;

    std::vector<std::shared_ptr<RangeSender>> objects =
    {
        std::make_shared<RangeSender>(1),
        std::make_shared<RangeSender>(2)
    };

    RangeChangedAccumulator accumulator(objects);
    std::vector<std::shared_ptr<RangeSender>> senders;
    std::vector<int> values;

    awl::coro::Job job = waitAccumulatedValues(accumulator, senders, values);

    AWL_ASSERT(!job.done());

    objects[0]->changed.emit(67);

    AWL_ASSERT(!job.done());
    AWL_ASSERT_EQUAL(1u, senders.size());
    AWL_ASSERT(senders[0] == objects[0]);
    AWL_ASSERT_EQUAL(67, values[0]);

    objects[1]->changed.emit(71);

    AWL_ASSERT(job.done());
    AWL_ASSERT_EQUAL(2u, senders.size());
    AWL_ASSERT(senders[1] == objects[1]);
    AWL_ASSERT_EQUAL(71, values[1]);
    AWL_ASSERT_EQUAL(1u, objects[0]->changed.size());
    AWL_ASSERT_EQUAL(1u, objects[1]->changed.size());
}

AWL_TEST(SignalRangeAccumulator_ObservableSetAdded)
{
    AWL_UNUSED_CONTEXT;

    awl::observable_vector_set<std::shared_ptr<RangeSender>> objects;
    std::shared_ptr<RangeSender> sender;
    int value = 0;

    awl::coro::Job job = waitRangeValue(objects, sender, value);

    AWL_ASSERT(!job.done());

    std::shared_ptr<RangeSender> object = std::make_shared<RangeSender>(1);
    objects.insert(object);

    AWL_ASSERT_EQUAL(1u, object->changed.size());

    object->changed.emit(41);

    AWL_ASSERT(job.done());
    AWL_ASSERT(sender == object);
    AWL_ASSERT_EQUAL(41, value);
    AWL_ASSERT(object->changed.empty());
}

AWL_TEST(SignalRangeAccumulator_ObservableUnorderedSetAdded)
{
    AWL_UNUSED_CONTEXT;

    awl::observable_unordered_set<std::shared_ptr<RangeSender>> objects;
    std::shared_ptr<RangeSender> sender;
    int value = 0;

    awl::coro::Job job = waitRangeValue(objects, sender, value);

    AWL_ASSERT(!job.done());

    std::shared_ptr<RangeSender> object = std::make_shared<RangeSender>(1);
    objects.insert(object);

    AWL_ASSERT_EQUAL(1u, object->changed.size());

    object->changed.emit(43);

    AWL_ASSERT(job.done());
    AWL_ASSERT(sender == object);
    AWL_ASSERT_EQUAL(43, value);
    AWL_ASSERT(object->changed.empty());
}

AWL_TEST(SignalRangeAccumulator_ObservableSetRemoving)
{
    AWL_UNUSED_CONTEXT;

    awl::observable_vector_set<std::shared_ptr<RangeSender>> objects;
    std::shared_ptr<RangeSender> first = std::make_shared<RangeSender>(1);
    std::shared_ptr<RangeSender> second = std::make_shared<RangeSender>(2);

    objects.insert(first);
    objects.insert(second);

    std::shared_ptr<RangeSender> sender;
    int value = 0;

    awl::coro::Job job = waitRangeValue(objects, sender, value);

    AWL_ASSERT(!job.done());
    AWL_ASSERT_EQUAL(1u, first->changed.size());
    AWL_ASSERT_EQUAL(1u, second->changed.size());

    objects.erase(first);

    AWL_ASSERT(first->changed.empty());
    AWL_ASSERT_EQUAL(1u, second->changed.size());

    first->changed.emit(45);

    AWL_ASSERT(!job.done());
    AWL_ASSERT(sender == nullptr);

    second->changed.emit(47);

    AWL_ASSERT(job.done());
    AWL_ASSERT(sender == second);
    AWL_ASSERT_EQUAL(47, value);
    AWL_ASSERT(second->changed.empty());
}

AWL_TEST(SignalRangeAccumulator_ObservableSetClearing)
{
    AWL_UNUSED_CONTEXT;

    awl::observable_vector_set<std::shared_ptr<RangeSender>> objects;
    std::shared_ptr<RangeSender> first = std::make_shared<RangeSender>(1);
    std::shared_ptr<RangeSender> second = std::make_shared<RangeSender>(2);

    objects.insert(first);
    objects.insert(second);

    std::shared_ptr<RangeSender> sender;
    int value = 0;

    awl::coro::Job job = waitRangeValue(objects, sender, value);

    AWL_ASSERT(!job.done());

    objects.clear();

    AWL_ASSERT(first->changed.empty());
    AWL_ASSERT(second->changed.empty());

    first->changed.emit(51);
    second->changed.emit(53);

    AWL_ASSERT(!job.done());
    AWL_ASSERT(sender == nullptr);
    AWL_ASSERT_EQUAL(0, value);

    std::shared_ptr<RangeSender> third = std::make_shared<RangeSender>(3);
    objects.insert(third);

    AWL_ASSERT_EQUAL(1u, third->changed.size());

    third->changed.emit(55);

    AWL_ASSERT(job.done());
    AWL_ASSERT(sender == third);
    AWL_ASSERT_EQUAL(55, value);
    AWL_ASSERT(third->changed.empty());
}

AWL_TEST(SignalRangeAccumulator_ObservableSetUnsubscribesOnCancel)
{
    AWL_UNUSED_CONTEXT;

    awl::observable_vector_set<std::shared_ptr<RangeSender>> objects;
    std::shared_ptr<RangeSender> sender;
    int value = 0;

    {
        awl::coro::Job job = waitRangeValue(objects, sender, value);

        AWL_ASSERT(!job.done());
    }

    std::shared_ptr<RangeSender> object = std::make_shared<RangeSender>(1);
    objects.insert(object);

    AWL_ASSERT(object->changed.empty());

    object->changed.emit(57);

    AWL_ASSERT(sender == nullptr);
    AWL_ASSERT_EQUAL(0, value);
}
