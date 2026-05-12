#include "Awl/Coro/SignalAwaitable.h"
#include "Awl/Coro/Job.h"
#include "Awl/Testing/UnitTest.h"

#include <memory>
#include <string>

namespace
{
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
