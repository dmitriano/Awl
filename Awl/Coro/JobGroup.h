#pragma once

#include "Awl/Coro/TaskSink.h"
#include "Awl/Coro/Job.h"
#include "Awl/Observable.h"

#include <vector>
namespace awl
{
    class JobGroup : public Observable<TaskSink>
    {
    private:

        class AnyAwaitable : private Observer<TaskSink>
        {
        public:

            AnyAwaitable(Observable<TaskSink>& source)
            {
                source.subscribe(this);
            }

            bool await_ready()
            {
                return _any;
            }

            // h is a handler for current coroutine which is suspended
            void await_suspend(std::coroutine_handle<> h)
            {
                _h = h;
            }

            void await_resume() {}

        private:

            void onFinished() override
            {
                _any = true;

                _h.resume();
            }

            bool _any = false;

            std::coroutine_handle<> _h;
        };

        friend AnyAwaitable;

        class AllAwaitable : private Observer<TaskSink>
        {
        public:

            AllAwaitable(JobGroup* p_this) : pThis(p_this)
            {
                pThis->subscribe(this);
            }

            bool await_ready()
            {
                return empty();
            }

            // h is a handler for current coroutine which is suspended
            void await_suspend(std::coroutine_handle<> h)
            {
                _h = h;
            }

            void await_resume() {}

        private:

            void onFinished() override
            {
                if (empty())
                {
                    _h.resume();
                }
            }

            bool empty() const
            {
                return pThis->empty();
            }

            JobGroup* const pThis;

            std::coroutine_handle<> _h;
        };

        friend AllAwaitable;

    public:

        JobGroup() = default;

        JobGroup(const JobGroup& other) = delete;

        JobGroup(JobGroup&& other) noexcept;

        JobGroup& operator = (const JobGroup& other) = delete;

        JobGroup& operator = (JobGroup&& other) noexcept;

        void spawn(Job&& task);

        std::size_t task_count() const
        {
            return _handlers.size();
        }

        bool empty() const
        {
            return _handlers.empty();
        }

        // Clears tracked jobs and wakes awaiters.
        void cancel();

        auto wait_all()
        {
            return AllAwaitable{ this };
        }

        auto wait_any()
        {
            return AnyAwaitable{ *this };
        }

    private:

        void updateHandlersOwner();

        Job wait_all_jobs_experimental();

        friend class JobGroupTestAccess;

        // Handlers do not need virtual destructor.
        struct Handler final : Observer<TaskSink>
        {
            Handler(JobGroup* p_this, Job&& task) :
                pThis(p_this),
                _task(std::move(task))
            {}

            JobGroup* pThis;

            Job _task;

            void onFinished() override;
        };

        friend Handler;

        // The tasks remove themselves automatically from the vector
        // when their promises are destroyed.
        std::vector<Handler> _handlers;
    };
}
