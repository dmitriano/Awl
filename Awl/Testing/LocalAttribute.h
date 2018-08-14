#pragma once

#include "Awl/Testing/AttributeProvider.h"

namespace awl
{
    namespace testing
    {
        template <class T>
        class LocalAttribute : public AttributeParser
        {
        public:

            LocalAttribute(const AttributeProvider & provider, const Char * type, const Char * name, T default_val) : AttributeParser(name), myType(type), defaultVal(default_val)
            {
                if (!provider.TryGet(*this))
                {
                    myVal = defaultVal;
                }
            }

            T GetValue() const
            {
                return myVal;
            }

            bool Parse(const String & s) override
            {
                try
                {
                    awl::FromString(s, myVal);
                }
                catch (const std::exception &)
                {
                    return false;
                }

                return true;
            }

            String GetDefaultValue() const override
            {
                return ToString(myVal);
            }

            String GetTypeName() const override
            {
                return myType;
            }

        protected:

            const Char * myType;

            T myVal;
            
            T defaultVal;
        };
    }
}

#define AWL_ATTRIBUTE(attribute_type, attribute_name, default_val) const attribute_type attribute_name = LocalAttribute<attribute_type>(context.ap, _T(#attribute_type), _T(#attribute_name), default_val).GetValue()
