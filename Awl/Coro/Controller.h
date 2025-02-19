#pragma once

#include "Awl/Coro/TaskSink.h"

#include "Awl/Coro/UpdateTask.h"
#include "Awl/Observable.h"

#include <vector>

namespace awl
{
    class Controller
    {
    public:

        void register_task(UpdateTask&& task);

        std::size_t task_count() const
        {
            return m_handlers.size();
        }

        void cancel();

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
