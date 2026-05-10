/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/SequentialStream.h"
#include "Awl/Io/IoException.h"
#include "Awl/DataCast.h"

namespace awl 
{
    namespace io
    {
        class StdInputStream : public SequentialInputStream
        {
        public:

            StdInputStream(std::istream & in) : _in(in)
            {}

            size_t read(uint8_t * buffer, size_t count) override
            {
                //good() returns false at the end of file.
                _in.read(launder_cast<char>(buffer), count);

                const size_t actually_read = _in.gcount();

                return actually_read;
            }

            bool end() override
            {
                _in.peek();

                return _in.eof();
            }

        protected:

            std::istream & _in;
        };

        class StdOutputStream : public SequentialOutputStream
        {
        public:

            StdOutputStream(std::ostream & out) : _out(out)
            {}

            void write(const uint8_t * buffer, size_t count) override
            {
                //he write function returns the stream itself. So in your case, it will return a reference to myfile.
                //The stream types are convertible to bool to check its failure status.
                
                if (!_out.write(launder_cast<const char>(buffer), count))
                {
                    throw WriteFailException();
                }
            }

        protected:

            std::ostream & _out;
        };
    }
}
