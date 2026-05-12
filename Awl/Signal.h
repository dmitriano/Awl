/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/EquatableFunction.h"
#include "Awl/UniqueId.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <concepts>
#include <cstdint>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

namespace awl
{
    template <class... Args>
    class Signal
    {
    public:

        using Slot = equatable_function<void(Args...)>;

        // Signal is a non-owning subscription interface. It intentionally has no virtual destructor:
        // only the owner deletes the concrete SignalSource instance.

        Id subscribe(std::function<void(Args...)> func)
        {
            const Id id = unique_id();
            subscribe(Slot(id, std::move(func)));
            return id;
        }

        virtual void subscribe(Slot slot) = 0;

        template <class Object>
        void subscribe(Object* p_object, void (Object::*member)(Args...))
        {
            subscribe(Slot(p_object, member));
        }

        template <class Object>
        void subscribe(const Object* p_object, void (Object::*member)(Args...) const)
        {
            subscribe(Slot(p_object, member));
        }

        template <class Object>
        void subscribe(std::shared_ptr<Object> p_object, void (Object::*member)(Args...))
        {
            subscribe(Slot(std::move(p_object), member));
        }

        template <class Object>
        void subscribe(std::shared_ptr<Object> p_object, void (Object::*member)(Args...) const)
        {
            subscribe(Slot(std::move(p_object), member));
        }

        template <class Object>
        void subscribe(std::weak_ptr<Object> p_object, void (Object::*member)(Args...))
        {
            subscribe(Slot(std::move(p_object), member));
        }

        template <class Object>
        void subscribe(std::weak_ptr<Object> p_object, void (Object::*member)(Args...) const)
        {
            subscribe(Slot(std::move(p_object), member));
        }

        virtual bool unsubscribe(const Slot& slot) = 0;

        bool unsubscribe(Id id)
        {
            return unsubscribe(Slot(id, std::function<void(Args...)>{ [](Args...) {} }));
        }

        template <class Object>
        bool unsubscribe(Object* p_object, void (Object::*member)(Args...))
        {
            return unsubscribe(Slot(p_object, member));
        }

        template <class Object>
        bool unsubscribe(const Object* p_object, void (Object::*member)(Args...) const)
        {
            return unsubscribe(Slot(p_object, member));
        }

        template <class Object>
        bool unsubscribe(std::shared_ptr<Object> p_object, void (Object::*member)(Args...))
        {
            return unsubscribe(Slot(std::move(p_object), member));
        }

        template <class Object>
        bool unsubscribe(std::shared_ptr<Object> p_object, void (Object::*member)(Args...) const)
        {
            return unsubscribe(Slot(std::move(p_object), member));
        }

        template <class Object>
        bool unsubscribe(std::weak_ptr<Object> p_object, void (Object::*member)(Args...))
        {
            return unsubscribe(Slot(std::move(p_object), member));
        }

        template <class Object>
        bool unsubscribe(std::weak_ptr<Object> p_object, void (Object::*member)(Args...) const)
        {
            return unsubscribe(Slot(std::move(p_object), member));
        }

        virtual bool empty() const noexcept = 0;

        virtual std::size_t size() const noexcept = 0;
    };

    template <class... Args>
    class SignalSource :
        public Signal<Args...>
    {
    private:

        using Slot = typename Signal<Args...>::Slot;

        struct SlotRecord
        {
            Slot slot;
            bool removed = false;
        };

    public:

        using container_type = std::vector<SlotRecord>;
        using Signal<Args...>::subscribe;
        using Signal<Args...>::unsubscribe;

        void subscribe(Slot slot) override
        {
            const auto it = std::find_if(_slots.begin(), _slots.end(),
                [&slot](const SlotRecord& record)
                {
                    return !record.removed && record.slot == slot;
                });

            if (it == _slots.end())
            {
                _slots.push_back(SlotRecord{ std::move(slot) });
            }
        }

        bool unsubscribe(const Slot& slot) override
        {
            const auto it = std::find_if(_slots.begin(), _slots.end(),
                [&slot](const SlotRecord& record)
                {
                    return !record.removed && record.slot == slot;
                });

            if (it == _slots.end())
            {
                return false;
            }

            if (_emitting)
            {
                it->removed = true;
            }
            else
            {
                eraseRecord(it);
            }

            return true;
        }

        template<typename ...Params>
        void emit(const Params&... args) const
            requires (std::invocable<Slot&, const Params&...>)
        {
            assert(!_emitting);

            EmitGuard guard(*this);

            for (SlotRecord& record : _slots)
            {
                if (record.removed)
                {
                    continue;
                }

                auto slot_guard = record.slot.lock();

                if (slot_guard)
                {
                    slot_guard(args...);
                }
                else
                {
                    record.removed = true;
                }
            }
        }

        void clear() noexcept
        {
            container_type().swap(_slots);
        }

        bool empty() const noexcept override
        {
            return size() == 0;
        }

        std::size_t size() const noexcept override
        {
            return static_cast<std::size_t>(std::count_if(_slots.begin(), _slots.end(),
                [](const SlotRecord& record)
                {
                    return !record.removed;
                }));
        }

    private:

        class EmitGuard
        {
        public:

            explicit EmitGuard(const SignalSource& signal) :
                _signal(signal)
            {
                _signal._emitting = true;
            }

            ~EmitGuard()
            {
                _signal._emitting = false;
                _signal.compact();
            }

        private:

            const SignalSource& _signal;
        };

        void eraseRecord(container_type::iterator it)
        {
            auto last = _slots.end();
            --last;

            if (it != last)
            {
                *it = std::move(*last);
            }

            _slots.pop_back();
        }

        void compact() const
        {
            std::erase_if(_slots,
                [](const SlotRecord& record)
                {
                    return record.removed;
                });
        }

        mutable bool _emitting = false;
        mutable container_type _slots;
    };
}
