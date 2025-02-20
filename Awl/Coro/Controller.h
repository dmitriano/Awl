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

        class AllAwaitable : private Observer<TaskSink>
        {
        public:

            AllAwaitable(Controller* p_this) : pThis(p_this)
            {
                pThis->Subscribe(this);
            }

            bool await_ready()
            {
                return empty();
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
                if (empty())
                {
                    m_h.resume();
                }
            }

            bool empty() const
            {
                return pThis->empty();
            }

            Controller* const pThis;

            std::coroutine_handle<> m_h;
        };

        friend AllAwaitable;

    public:

        void register_task(UpdateTask&& task);

        std::size_t task_count() const
        {
            return m_handlers.size();
        }

        bool empty() const
        {
            return m_handlers.empty();
        }

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

        UpdateTask wait_all_task_experimental();

        friend class ControllerTest;

        struct Handler : Observer<TaskSink>
        {
            Handler(Controller* p_this, UpdateTask&& task) :
                pThis(p_this),
                m_task(std::move(task))
            {}

            Controller* pThis;

            UpdateTask m_task;

            void OnFinished() override;
        };

        friend Handler;

        // The tasks remove themself automatically from the vector
        // when their promises are destroyed.
        std::vector<Handler> m_handlers;
    };
}
