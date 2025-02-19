#pragma once

#include "Awl/Coro/UpdatePromise.h"
#include "Awl/Coro/UpdateTask.h"

using namespace awl;

UpdateTask UpdatePromise::get_return_object()
{
    return { std::coroutine_handle<UpdatePromise>::from_promise(*this) };
}
