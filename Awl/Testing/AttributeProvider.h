#pragma once

#include "Awl/String.h"

namespace awl
{
    namespace testing
    {
        class NamedParser
        {
        public:

            NamedParser(const Char * name) : myName(name)
            {
            }

            const Char * GetName() const
            {
                return myName;
            }

            virtual ~NamedParser() = default;

        protected:

             virtual bool Parse(const String & s) = 0;

        private:

            const Char * myName;

            friend class AttributeProvider;
        };

        class AttributeProvider
        {
        public:

            bool TryGet(NamedParser & parser) const
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
