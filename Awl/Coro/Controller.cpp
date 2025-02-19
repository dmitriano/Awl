#pragma once

#include "Awl/Coro/Controller.h"

#include <cassert>

using namespace awl;

void Controller::register_task(UpdateTask&& task)
{
    UpdateTask::promise_type& promise = task.m_h.promise();

    // The promise is owned at this point.
    assert(task.m_h != nullptr);

    m_promises.push_back(std::addressof(promise));

    task.release();
}

void Controller::cancel()
{
    while (!m_promises.empty())
    {
        UpdateTask::promise_type* p_promise = m_promises.pop_front();
        
        auto h = std::coroutine_handle<UpdateTask::promise_type>::from_promise(*p_promise);

        h.destroy();
    }
}
