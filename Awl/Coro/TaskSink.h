#pragma once

namespace awl
{
    class TaskSink
    {
    public:

        virtual void onFinished() = 0;
    };

    template <class Key, class Value>
    class MappedTaskSink
    {
    public:

        virtual void onFinished(const Key& key, const Value& value) = 0;
    };
}
