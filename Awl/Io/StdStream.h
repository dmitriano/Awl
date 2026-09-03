/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/SequentialStream.h"
#include "Awl/Io/IoException.h"
#include "Awl/DataCast.h"

#include <cassert>
#include <istream>
#include <limits>
#include <ostream>

namespace awl 
{
    namespace io
    {
        namespace detail
        {
            inline std::streamsize toStreamSize(const size_t count)
            {
                constexpr size_t max_stream_size = static_cast<size_t>((std::numeric_limits<std::streamsize>::max)());

                if (count > max_stream_size)
                {
                    throw IoError(_T("Stream block is too large."));
                }

                return static_cast<std::streamsize>(count);
            }

            inline size_t fromStreamSize(const std::streamsize count)
            {
                assert(count >= 0);

                return static_cast<size_t>(count);
            }
        }

        class StdInputStream : public SequentialInputStream
        {
        public:

            StdInputStream(std::istream & in) : _in(in)
            {}

            size_t read(std::byte * buffer, size_t count) override
            {
                //good() returns false at the end of file.
                _in.read(char_cast(buffer), detail::toStreamSize(count));

                const size_t actually_read = detail::fromStreamSize(_in.gcount());

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

            void write(const std::byte * buffer, size_t count) override
            {
                //he write function returns the stream itself. So in your case, it will return a reference to myfile.
                //The stream types are convertible to bool to check its failure status.
                
                if (!_out.write(char_cast(buffer), detail::toStreamSize(count)))
                {
                    throw WriteFailException();
                }
            }

        protected:

            std::ostream & _out;
        };
    }
}
