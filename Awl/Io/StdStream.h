#pragma once

#include "Awl/Io/SequentialStream.h"
#include "Awl/Io/IoException.h"

namespace awl 
{
    namespace io
    {
        class StdInputStream : public SequentialInputStream
        {
        public:

            StdInputStream(std::istream & in) : m_in(in)
            {
            }

            size_t Read(uint8_t * buffer, size_t count) override
            {
                //good() returns false at the end of file.
                m_in.read(reinterpret_cast<char *>(buffer), count);

                const size_t actually_read = m_in.gcount();

                return actually_read;
            }

            bool End() override
            {
                m_in.peek();

                return m_in.eof();
            }

        protected:

            std::istream & m_in;
        };

        class StdOutputStream : public SequentialOutputStream
        {
        public:

            StdOutputStream(std::ostream & out) : m_out(out)
            {
            }

            void Write(const uint8_t * buffer, size_t count) override
            {
                //he write function returns the stream itself. So in your case, it will return a reference to myfile.
                //The stream types are convertible to bool to check its failure status.
                
                if (!m_out.write(reinterpret_cast<const char *>(buffer), count))
                {
                    throw WriteFailException();
                }
            }

        protected:

            std::ostream & m_out;
        };
    }
}
