/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <memory>
#include <type_traits>

namespace awl
{
    template <class T> requires std::is_default_constructible_v<T>
    std::shared_ptr<T> shared_singleton()
    {
        static std::weak_ptr<T> wp;

        std::shared_ptr<T> p = wp.lock();

        if (p == nullptr)
        {
            p = std::make_shared<T>();

            wp = p;
        }

        return p;
    }
}
