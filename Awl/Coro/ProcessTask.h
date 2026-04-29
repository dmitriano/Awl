#pragma once

#include "Awl/Coro/Task.h"

namespace awl
{
    template <class T>
    using ProcessTask = Task<T>;
}
