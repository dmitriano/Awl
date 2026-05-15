/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Helpers/DelayedAwaitable.h"
#include "Awl/Coro/Job.h"
#include "Awl/Coro/JobGroup.h"
#include "Helpers/TimeQueue.h"
#include "Awl/Testing/UnitTest.h"

#include <type_traits>

static_assert(!std::is_copy_constructible_v<awl::JobGroup>);
static_assert(!std::is_copy_assignable_v<awl::JobGroup>);
static_assert(std::is_move_constructible_v<awl::JobGroup>);
static_assert(std::is_move_assignable_v<awl::JobGroup>);

namespace
{
    using namespace std::chrono_literals;

    awl::Job delayedJob(
        const awl::testing::TestContext& context,
        awl::testing::IDelayedExecutor& delayed_executor,
        int id)
    {
        co_await awl::testing::DelayedAwaitable(delayed_executor, 100ms);

        context.logger->debug(_T("{} finished"), id);
    }

    void spawnThreeJobs(
        const awl::testing::TestContext& context,
        awl::testing::IDelayedExecutor& delayed_executor,
        awl::JobGroup& jobs)
    {
        jobs.spawn(delayedJob(context, delayed_executor, 1));
        jobs.spawn(delayedJob(context, delayed_executor, 2));
        jobs.spawn(delayedJob(context, delayed_executor, 3));

        AWL_ASSERT_EQUAL(3u, jobs.task_count());
    }
}

namespace awl
{
    class JobGroupTestAccess
    {
    public:

        static awl::Job waitAllJobs(
            const awl::testing::TestContext& context,
            awl::testing::IDelayedExecutor& delayed_executor,
            awl::JobGroup& jobs)
        {
            spawnThreeJobs(context, delayed_executor, jobs);

            co_await jobs.wait_all_jobs_experimental();
        }

        static awl::Job waitAnyThenWaitAll(
            const awl::testing::TestContext& context,
            awl::testing::IDelayedExecutor& delayed_executor,
            awl::JobGroup& jobs,
            bool wait_jobs_directly = false,
            std::size_t expected_count_after_wait_any = 2)
        {
            spawnThreeJobs(context, delayed_executor, jobs);

            context.logger->debug("wait_any() started");

            co_await jobs.wait_any();

            context.logger->debug("wait_any() finished");

            AWL_ASSERT_EQUAL(expected_count_after_wait_any, jobs.task_count());

            if (wait_jobs_directly)
            {
                co_await jobs.wait_all_jobs_experimental();
            }
            else
            {
                co_await jobs.wait_all();
            }

            context.logger->debug("wait_all() finished");

            AWL_ASSERT_EQUAL(0u, jobs.task_count());
        }
    };
}

AWL_TEST(JobGroupCancel)
{
    awl::testing::TimeQueue time_queue;
    awl::JobGroup jobs;

    jobs.spawn(delayedJob(context, time_queue, 1));

    AWL_ASSERT_EQUAL(1u, jobs.task_count());

    jobs.cancel();

    AWL_ASSERT_EQUAL(0u, jobs.task_count());

    time_queue.clear();
}

AWL_TEST(JobGroupRemovesFinishedJobs)
{
    awl::testing::TimeQueue time_queue;
    awl::JobGroup jobs;

    jobs.spawn(delayedJob(context, time_queue, 1));

    AWL_ASSERT_EQUAL(1u, jobs.task_count());

    time_queue.loop();

    AWL_ASSERT_EQUAL(0u, jobs.task_count());
}

AWL_TEST(JobGroupWaitAllJobsDirectly)
{
    awl::testing::TimeQueue time_queue;
    awl::JobGroup jobs;

    awl::Job task = awl::JobGroupTestAccess::waitAllJobs(context, time_queue, jobs);

    time_queue.loop(1);

    AWL_ASSERT_EQUAL(2u, jobs.task_count());

    time_queue.loop(1);

    AWL_ASSERT_EQUAL(1u, jobs.task_count());

    time_queue.loop(1);

    AWL_ASSERT_EQUAL(0u, jobs.task_count());

    AWL_ASSERT(time_queue.empty());

    AWL_ASSERT(task.done());
}

AWL_TEST(JobGroupWaitAnyThenWaitAll)
{
    awl::testing::TimeQueue time_queue;
    awl::JobGroup jobs;

    awl::Job task = awl::JobGroupTestAccess::waitAnyThenWaitAll(context, time_queue, jobs);

    time_queue.loop();

    AWL_ASSERT(task.done());
}

AWL_TEST(JobGroupMoveConstruct)
{
    awl::testing::TimeQueue time_queue;
    awl::JobGroup jobs;

    spawnThreeJobs(context, time_queue, jobs);

    awl::JobGroup moved_jobs(std::move(jobs));

    AWL_ASSERT_EQUAL(3u, moved_jobs.task_count());

    time_queue.loop();

    AWL_ASSERT_EQUAL(0u, moved_jobs.task_count());
}

AWL_TEST(JobGroupMoveAssign)
{
    awl::testing::TimeQueue time_queue;
    awl::JobGroup jobs;
    awl::JobGroup moved_jobs;

    spawnThreeJobs(context, time_queue, jobs);

    moved_jobs = std::move(jobs);

    AWL_ASSERT_EQUAL(3u, moved_jobs.task_count());

    time_queue.loop();

    AWL_ASSERT_EQUAL(0u, moved_jobs.task_count());
}

AWL_TEST(JobGroupCancelBeforeWaiting)
{
    AWL_FLAG(wait_jobs_directly);

    awl::testing::TimeQueue time_queue;
    awl::JobGroup jobs;

    awl::Job task = awl::JobGroupTestAccess::waitAnyThenWaitAll(context, time_queue, jobs, wait_jobs_directly, 0);

    jobs.cancel();

    AWL_ASSERT(task.done());

    time_queue.clear();
}

// Fails with wait_jobs_directly=true.
// UB: Awl/Awl/Coro/JobGroup.cpp:62:5: runtime error: member access within address
// that does not point to an object of type 'Handler'.
AWL_TEST(JobGroupCancelAfterFirstJobFinished)
{
    AWL_FLAG(wait_jobs_directly);

    awl::testing::TimeQueue time_queue;
    awl::JobGroup jobs;

    awl::Job task = awl::JobGroupTestAccess::waitAnyThenWaitAll(context, time_queue, jobs, wait_jobs_directly);

    time_queue.loop(1);

    jobs.cancel();

    AWL_ASSERT(task.done());

    time_queue.clear();
}
