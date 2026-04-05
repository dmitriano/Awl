/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Exception.h"
#include "Awl/StringFormat.h"

#include <memory>
#include <typeindex>
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
            const std::type_index requested_type(typeid(Value));

            for (auto& p_value : m_values)
            {
                if (p_value->type() == requested_type)
                {
                    p_value = std::make_shared<Model<Value>>(val);
                    return;
                }
            }

            m_values.push_back(std::make_shared<Model<Value>>(val));
        }

        template <class T>
        bool tryGet(T& val) const
        {
            using Value = std::decay_t<T>;
            const std::type_index requested_type(typeid(Value));

            for (const auto& p_value : m_values)
            {
                if (p_value->type() != requested_type)
                {
                    continue;
                }

                auto p_model = std::dynamic_pointer_cast<Model<Value>>(p_value);

                if (p_model == nullptr)
                {
                    throw awl::GeneralException(awl::format() << "TypeProvider: bad stored type for " << typeid(Value).name() << ".");
                }

                val = p_model->val;

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

        struct IModel
        {
            virtual ~IModel() = default;

            virtual std::type_index type() const = 0;
        };

        template <class T>
        struct Model : public IModel
        {
            explicit Model(const T& init_val) :
                val(init_val)
            {}

            std::type_index type() const override
            {
                return std::type_index(typeid(T));
            }

            T val;
        };

        std::vector<std::shared_ptr<IModel>> m_values;
    };
}
