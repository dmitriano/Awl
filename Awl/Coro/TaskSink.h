#pragma once

#include <functional>

namespace awl
{
    class TaskSink
    {
    public:

        virtual void OnFinished() = 0;
    };

    template <class Key, class Value>
    using MappedTaskFinishedCallback = std::function<void(const Key& key, const Value& value)>;
}
