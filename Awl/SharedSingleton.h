/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <memory>
#include <type_traits>

namespace awl
{
    template <class T>
    bool is_uninitialized(const std::weak_ptr<T>& wp)
    {
        using WeakPtr = std::weak_ptr<T>;
        return !wp.owner_before(WeakPtr{}) && !WeakPtr{}.owner_before(wp);
    }

    //Let the user define a function that crates its singleton instance.
    template <class T>
    std::shared_ptr<T> make_singleton_instance();

    template <class T> requires std::is_default_constructible_v<T>
    std::shared_ptr<T> make_singleton_instance()
    {
        //We have a long leaving std::weak_ptr so we may not use std::make_shared.
        return std::make_shared<T>();
    }

    //Does not recreate the instance after it was destroyed, but returns nullptr.
    template <class T>
    std::shared_ptr<T> shared_singleton()
    {
        static std::weak_ptr<T> wp;

        if (is_uninitialized(wp))
        {
            std::shared_ptr<T> p = make_singleton_instance<T>();

            wp = p;

            return p;
        }

        return wp.lock();
    }

    //Alternative implementation that recreates the instance.
    template <class T>
    std::shared_ptr<T> ondemand_singleton()
    {
        static std::weak_ptr<T> wp;

        std::shared_ptr<T> p = wp.lock();

        if (p == nullptr)
        {
            //We have a long leaving std::weak_ptr so we may not use std::make_shared.
            p = make_singleton_instance<T>();

            wp = p;
        }

        return p;
    }
}
