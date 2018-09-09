#pragma once

#include <cstdint>
#include <iostream>

namespace awl 
{
    namespace io
    {
        class SequentialInputStream
        {
        public:

            virtual bool End() = 0;
                
            virtual size_t Read(uint8_t * buffer, size_t count) = 0;

            virtual ~SequentialInputStream() = default;
        };

        class SequentialOutputStream
        {
        public:

            virtual void Write(const uint8_t * buffer, size_t count) = 0;

            virtual ~SequentialOutputStream() = default;
        };

        class StdInputStream : public SequentialInputStream
        {
        public:

            StdInputStream(std::istream & in) : m_in(in)
            {
            }

            size_t Read(uint8_t * buffer, size_t count) override
            {
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
    }
}
