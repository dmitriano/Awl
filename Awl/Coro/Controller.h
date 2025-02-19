#pragma once

#include "Awl/Coro/UpdateTask.h"
#include "Awl/QuickList.h"

namespace awl
{
    class Controller
    {
    public:

        void register_task(UpdateTask&& task);

        std::size_t task_count() const
        {
            return m_promises.size();
        }

        void cancel();

    private:

        // The tasks removes themself automatically from the list
        // when their promises are destroyed.
        awl::quick_list<UpdatePromise, ControllerLink> m_promises;
    };
}
