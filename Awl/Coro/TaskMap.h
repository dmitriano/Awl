#pragma once

#include "Awl/Coro/JobGroup.h"
#include "Awl/Coro/TaskSink.h"
#include "Awl/Coro/Job.h"
#include "Awl/Observable.h"
#include "Awl/KeyCompare.h"
#include "Awl/MemFn.h"

#include <vector>

namespace awl
{
    template <class Key, class Value>
    class TaskMap : public awl::Observable<MappedTaskSink<Key, Value>>
    {
    private:

        // Handlers do not need virtual destructor.
        struct Handler final : Observer<TaskSink>
        {
            Handler(TaskMap* p_this, Key key, Value value) :
                pThis(p_this),
                _key(std::move(key)),
                _value(std::move(value))
            {}

            TaskMap* pThis;

            Key _key;

            Value _value;

            void onFinished() override
            {
                const std::size_t index = this - pThis->_handlers.data();

                assert(index < pThis->_handlers.size());

                Key temp_key = std::move(_key);

                Value temp_value = std::move(_value);

                pThis->_handlers.erase(pThis->_handlers.begin() + index);

                pThis->notify(&MappedTaskSink<Key, Value>::onFinished, temp_key, temp_value);
            }
        };

        friend Handler;

    public:

        void spawn(Job&& task, Key key, Value value)
        {
            // A coroutine has executed as a regular function.
            if (!task.done())
            {
                assert(!contains(key));

                _handlers.emplace_back(this, std::move(key), std::move(value));

                Handler& handler = _handlers.back();

                task.subscribe(&handler);
            }

            _jobs.spawn(std::move(task));
        }

        std::size_t task_count() const
        {
            return _jobs.task_count();
        }

        bool empty() const
        {
            return _jobs.empty();
        }

        void cancel()
        {
            _jobs.cancel();
        }

        auto wait_all()
        {
            return _jobs.wait_all();
        }

        auto wait_any()
        {
            return _jobs.wait_any();
        }

        const Value* find(const Key& key) const
        {
            auto i = std::ranges::find_if(_handlers, awl::mem_fn_equal_to(&Handler::_key, key));

            if (i == _handlers.end())
            {
                return nullptr;
            }

            return std::addressof(i->_value);
        }

        bool contains(const Key& key) const
        {
            return find(key) != nullptr;
        }

        auto keys() const
        {
            return _handlers | std::views::transform([](const Handler& h) -> const Key& { return h._key; });
        }

        auto elements() const
        {
            return _handlers | std::views::transform([](const Handler& h) { return std::make_pair(h._key, h._value); });
        }

    private:

        JobGroup _jobs;

        // The tasks remove themselves automatically from the vector
        // when their promises are destroyed.
        std::vector<Handler> _handlers;
    };
}
