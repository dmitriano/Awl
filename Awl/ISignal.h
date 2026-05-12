/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/EquatableFunction.h"
#include "Awl/UniqueId.h"

#include <functional>
#include <memory>
#include <utility>

namespace awl
{
    template <class... Args>
    class ISignal
    {
    public:

        using Slot = equatable_function<void(Args...)>;

        virtual ~ISignal() = default;

        virtual void subscribe(Slot slot) = 0;

        virtual bool unsubscribe(const Slot& slot) = 0;

        Id subscribe(std::function<void(Args...)> func)
        {
            const Id id = unique_id();
            subscribe(Slot(id, std::move(func)));
            return id;
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
    };
}
