/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Exception.h"

#include <algorithm>
#include <any>
#include <format>
#include <utility>
#include <vector>

namespace awl::testing
{
    class TypeProvider
    {
    public:

        template <class T>
        void set(T&& val)
        {
            using Value = std::decay_t<T>;

            const auto it = std::ranges::find(m_values, typeid(Value), &std::any::type);

            if (it != m_values.end())
            {
                *it = std::forward<T>(val);
                return;
            }

            m_values.emplace_back(std::forward<T>(val));
        }

        template <class T>
        T get() const
        {
            T val;

            if (!tryGet(val))
            {
                throw awl::GeneralException(std::format(_T("TypeProvider: type {} not found."),
                    awl::FromACString(typeid(std::decay_t<T>).name())));
            }

            return val;
        }

    private:

        template <class T>
        bool tryGet(T& val) const
        {
            using Value = std::decay_t<T>;

            const auto it = std::ranges::find(m_values, typeid(Value), &std::any::type);

            if (it != m_values.end())
            {
                val = std::any_cast<const Value&>(*it);
                return true;
            }

            return false;
        }

        std::vector<std::any> m_values;

        friend class TypeProviderTest;
    };
}
