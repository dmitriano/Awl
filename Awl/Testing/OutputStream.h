#pragma once

#include "Awl/String.h"

#include <sstream>

namespace awl 
{
    namespace testing 
    {
        class OutputStream
        {
        public:

            virtual void Write(const String & s) = 0;

            void EndLine()
            {
                Write(_T("\n"));
            }
        };
        
        class FakeOutputStream : public OutputStream
        {
        public:

            void Write(const String &) override
            {
            }
        };

        class StringOutputStream : public OutputStream
        {
        public:

            void Write(const String & s) override
            {
                out << s;
            }

            String GetString() const
            {
                return out.str();
            }
            
            void Clear()
            {
                out.clear();
            }

        private:

            std::basic_ostringstream<Char> out;
        };
    }
}
