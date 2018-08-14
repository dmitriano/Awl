#pragma once

#include "Awl/String.h"

namespace awl
{
    namespace testing
    {
        class AttributeParser
        {
        public:

            AttributeParser(const Char * name) : myName(name)
            {
            }

            const Char * GetName() const
            {
                return myName;
            }

            virtual ~AttributeParser() = default;

            virtual bool Parse(const String & s) = 0;

            virtual String GetDefaultValue() const = 0;

            virtual String GetTypeName() const = 0;

        private:

            const Char * myName;
        };

        class AttributeProvider
        {
        public:

            bool TryGet(AttributeParser & parser) const
            {
                String s;
                
                if (TryFind(s))
                {
                    return parser.Parse(s);
                }

                return false;
            }

        protected:

            virtual const bool TryFind(String & val) const
            {
                return false;
            }
        };
    }
}
