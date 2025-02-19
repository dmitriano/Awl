#pragma once

#include "Awl/Coro/TaskSink.h"

#include "Awl/Coro/UpdateTask.h"
#include "Awl/Observable.h"

#include <vector>

namespace awl
{
    class Controller : private Observable<TaskSink>
    {
    private:

        class AnyAwaitable : private Observer<TaskSink>
        {
        public:

            AnyAwaitable(Observable<TaskSink>& source)
            {
                source.Subscribe(this);
            }

            bool await_ready()
            {
                return m_any;
            }

            // h is a handler for current coroutine which is suspended
            void await_suspend(std::coroutine_handle<> h)
            {
                m_h = h;
            }

            void await_resume() {}

        private:

            void OnFinished() override
            {
                m_any = true;

                m_h.resume();
            }

            bool m_any = false;

            std::coroutine_handle<> m_h;
        };

        friend AnyAwaitable;

    public:

        void register_task(UpdateTask&& task);

        std::size_t task_count() const
        {
            return m_handlers.size();
        }

        void cancel();

        UpdateTask wait_all();

        auto wait_any()
        {
            return AnyAwaitable{*this};
        }

    private:

        struct Handler : Observer<TaskSink>
        {
            Handler(Controller* p_this, UpdateTask&& task) :
                pThis(p_this),
                m_task(std::move(task))
            {}

            UpdateTask m_task;

            Controller* pThis;

            void OnFinished() override;
        };

        friend Handler;

        // The tasks remove themself automatically from the vector
        // when their promises are destroyed.
        std::vector<Handler> m_handlers;
    };
}
