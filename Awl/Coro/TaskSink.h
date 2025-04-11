#pragma once

namespace awl
{
    class TaskSink
    {
    public:

        virtual void OnFinished() = 0;
    };

    template <class Key, class Value>
    class MappedTaskSink
    {
    public:

        virtual void OnFinished(const Key& key, const Value& value) = 0;
    };
}
