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

    // Let the user customize each singleton factory independently.
    template <class T>
    std::shared_ptr<T> make_shared_instance();

    template <class T> requires std::is_default_constructible_v<T>
    std::shared_ptr<T> make_shared_instance()
    {
        return std::make_shared<T>();
    }

    template <class T>
    std::shared_ptr<T> make_weak_instance();

    template <class T> requires std::is_default_constructible_v<T>
    std::shared_ptr<T> make_weak_instance()
    {
        // Allocate separately so the static weak_ptr does not retain the object's storage.
        return std::shared_ptr<T>(new T());
    }

    //Does not recreate the instance after it was destroyed, but returns nullptr.
    template <class T>
    std::shared_ptr<T> shared_singleton()
    {
        static std::weak_ptr<T> wp;

        if (is_uninitialized(wp))
        {
            std::shared_ptr<T> p = make_shared_instance<T>();

            wp = p;

            return p;
        }

        return wp.lock();
    }

    //Alternative implementation that recreates the instance.
    template <class T>
    std::shared_ptr<T> weak_singleton()
    {
        static std::weak_ptr<T> wp;

        std::shared_ptr<T> p = wp.lock();

        if (p == nullptr)
        {
            p = make_weak_instance<T>();

            wp = p;
        }

        return p;
    }
}
