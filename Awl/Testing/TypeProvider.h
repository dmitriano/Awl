/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Exception.h"
#include "Awl/StringFormat.h"

#include <algorithm>
#include <any>
#include <vector>

namespace awl::testing
{
    class TypeProvider
    {
    public:

        template <class T>
        void set(const T& val)
        {
            using Value = std::decay_t<T>;

            const auto it = std::ranges::find(m_values, typeid(Value), &std::any::type);

            if (it != m_values.end())
            {
                *it = val;
                return;
            }

            m_values.emplace_back(val);
        }

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

        template <class T>
        T get() const
        {
            T val;

            if (!tryGet(val))
            {
                throw awl::GeneralException(awl::format() << "TypeProvider: type " << typeid(std::decay_t<T>).name() << " not found.");
            }

            return val;
        }

    private:

        std::vector<std::any> m_values;
    };
}
