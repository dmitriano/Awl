#pragma once

#include "Awl/Coro/UpdateTask.h"

using namespace awl;

UpdateTask UpdatePromise::get_return_object()
{
    return { std::coroutine_handle<UpdatePromise>::from_promise(*this) };
}

void UpdateTask::release()
{
    if (m_h)
    {
        m_h.promise().m_owned = false;

        m_h = nullptr;
    }
}
