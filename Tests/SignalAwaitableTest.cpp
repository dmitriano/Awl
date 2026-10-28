#include "Awl/Coro/SignalAwaitable.h"
#include "Awl/Coro/Job.h"
#include "Awl/Source.h"
#include "Awl/Testing/UnitTest.h"

#include <memory>
#include <string>
#include <utility>

namespace
{
    awl::coro::Job waitInt(awl::ISignal<int>& signal, int& result)
    {
        result = co_await awl::coro::wait_signal(signal);
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
