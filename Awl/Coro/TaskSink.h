#pragma once

namespace awl
{
    class TaskSink
    {
    public:

        virtual void OnFinished() = 0;
    };
}
