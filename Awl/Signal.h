/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/EquatableFunction.h"
#include "Awl/UniqueId.h"

#include <algorithm>
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
        using container_type = std::vector<Slot>;

        Id subscribe(std::function<void(Args...)> func)
        {
            const Id id = unique_id();
            subscribe(Slot(id, std::move(func)));
            return id;
        }

        void subscribe(Slot slot)
        {
            if (std::find(_slots.begin(), _slots.end(), slot) == _slots.end())
            {
                _slots.push_back(std::move(slot));
            }
        }

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

        bool unsubscribe(const Slot& slot)
        {
            const auto it = std::find(_slots.begin(), _slots.end(), slot);

            if (it == _slots.end())
            {
                return false;
            }

            auto last = _slots.end();
            --last;

            if (it != last)
            {
                *it = std::move(*last);
            }

            _slots.pop_back();
            return true;
        }

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

        template<typename ...Params>
        void emit(const Params&... args) const
            requires (std::invocable<Slot&, const Params&...>)
        {
            auto i = _slots.begin();

            while (i != _slots.end())
            {
                auto guard = i->lock();

                if (guard)
                {
                    guard(args...);
                    ++i;
                }
                else
                {
                    auto last = _slots.end();
                    --last;
                    const bool removing_last = (i == last);

                    if (!removing_last)
                    {
                        *i = std::move(*last);
                    }

                    _slots.pop_back();

                    if (removing_last)
                    {
                        i = _slots.end();
                    }
                }
            }
        }

        void clear() noexcept
        {
            container_type().swap(_slots);
        }

        bool empty() const noexcept
        {
            return _slots.empty();
        }

        std::size_t size() const noexcept
        {
            return _slots.size();
        }

    private:

        mutable container_type _slots;
    };
}
