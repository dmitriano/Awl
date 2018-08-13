#pragma once

#include "Awl/Testing/AttributeProvider.h"

namespace awl
{
    namespace testing
    {
        template <class T>
        class LocalAttribute : public NamedParser
        {
        public:

            LocalAttribute(const AttributeProvider & provider, const Char * name, T default_val) : NamedParser(name)
            {
                if (!provider.TryGet(*this))
                {
                    myVal = default_val;
                }
            }

            operator T () const
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

        protected:

            T myVal;
        };
    }
}

#define AWL_ATTRIBUTE(attribute_type, attribute_name, default_val) LocalAttribute<attribute_type> attribute_name(context.ap, _T(#attribute_name), default_val)

