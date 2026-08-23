/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Observer.h"
#include "Awl/RangeUtil.h"

#include <concepts>
#include <type_traits>

namespace awl
{
    //The argument is const probably because it can be 'const shared_ptr<A> &'.
    template <class T>
    struct INotifySetChanged
    {
        virtual void onAdded(const T& val) = 0;
        virtual void onRemoving(const T& val) = 0;
        virtual void onClearing() = 0;
    };

    template <class T>
    using set_change_observer_t = Observer<INotifySetChanged<typename std::remove_cvref_t<T>::value_type>>;

    template <class T>
    concept observable_set_like =
        requires { typename std::remove_cvref_t<T>::value_type; } &&
        requires(const T& set, set_change_observer_t<T>* p_observer)
        {
            { set.subscribe(p_observer) } -> std::same_as<void>;
            { set.unsubscribe(p_observer) } -> std::same_as<void>;
        };

    template <class T>
    concept observable_shared_ptr_set =
        awl::input_shared_ptr_range<T> &&
        observable_set_like<T>;
}
