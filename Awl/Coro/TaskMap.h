#pragma once

#pragma once

#include "Awl/Coro/TaskPool.h"
#include "Awl/Coro/TaskSink.h"
#include "Awl/Coro/UpdateTask.h"
#include "Awl/Observable.h"
#include "Awl/KeyCompare.h"

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
                m_key(std::move(key)),
                m_value(std::move(value))
            {}

            TaskMap* pThis;

            Key m_key;

            Value m_value;

            void OnFinished() override
            {
                const std::size_t index = this - pThis->m_handlers.data();

                assert(index < pThis->m_handlers.size());

                Key temp_key = std::move(m_key);

                Value temp_value = std::move(m_value);

                pThis->m_handlers.erase(pThis->m_handlers.begin() + index);

                pThis->notify(&MappedTaskSink<Key, Value>::OnFinished, temp_key, temp_value);
            }
        };

        friend Handler;

    public:

        void spawn(UpdateTask&& task, Key key, Value value)
        {
            // A couroutine has executed as a regular function.
            if (!task.done())
            {
                assert(!contains(key));

                m_handlers.emplace_back(this, std::move(key), std::move(value));

                Handler& handler = m_handlers.back();

                task.subscribe(&handler);
            }

            m_pool.spawn(std::move(task));
        }

        std::size_t task_count() const
        {
            return m_pool.task_count();
        }

        bool empty() const
        {
            return m_pool.empty();
        }

        void cancel()
        {
            m_pool.cancel();
        }

        auto wait_all()
        {
            return m_pool.wait_all();
        }

        auto wait_any()
        {
            return m_pool.wait_any();
        }

        const Value* find(const Key& key) const
        {
            auto i = std::ranges::find_if(m_handlers, awl::mem_fn_equal_to(&Handler::m_key, key));

            if (i == m_handlers.end())
            {
                return nullptr;
            }

            return std::addressof(i->m_value);
        }

        bool contains(const Key& key) const
        {
            return find(key) != nullptr;
        }

        auto keys() const
        {
            return m_handlers | std::views::transform([](const Handler& h) -> const Key& { return h.m_key; });
        }

        auto elements() const
        {
            return m_handlers | std::views::transform([](const Handler& h) { return std::make_pair(h.m_key, h.m_value); });
        }

    private:

        TaskPool m_pool;

        // The tasks remove themself automatically from the vector
        // when their promises are destroyed.
        std::vector<Handler> m_handlers;
    };
}
