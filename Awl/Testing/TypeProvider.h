/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Exception.h"
#include "Awl/StringFormat.h"

#include <memory>
#include <typeindex>
#include <unordered_map>

namespace awl::testing
{
    class TypeProvider
    {
    public:

        template <class T>
        void set(const T& val)
        {
            using Value = std::decay_t<T>;

            m_values[std::type_index(typeid(Value))] = std::make_shared<Model<Value>>(val);
        }

        template <class T>
        bool tryGet(T& val) const
        {
            using Value = std::decay_t<T>;

            auto i = m_values.find(std::type_index(typeid(Value)));

            if (i == m_values.end())
            {
                return false;
            }

            auto p_model = std::dynamic_pointer_cast<Model<Value>>(i->second);

            if (p_model == nullptr)
            {
                throw awl::GeneralException(awl::format() << "TypeProvider: bad stored type for " << typeid(Value).name() << ".");
            }

            val = p_model->val;

            return true;
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

        struct IModel
        {
            virtual ~IModel() = default;
        };

        template <class T>
        struct Model : public IModel
        {
            explicit Model(const T& init_val) :
                val(init_val)
            {}

            T val;
        };

        std::unordered_map<std::type_index, std::shared_ptr<IModel>> m_values;
    };

    static_assert(attribute_provider<TypeProvider>);
}
