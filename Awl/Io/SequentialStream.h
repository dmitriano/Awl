#pragma once

#include <cstdint>

namespace awl 
{
    namespace io
    {
        class IoException
        {
        };

        class EndOfFileException : public IoException
        {
        };

        class WriteException : public IoException
        {
        };

        class SequentialInputStream
        {
        public:

            virtual bool End() = 0;
                
            virtual void Read(uint8_t * buffer, size_t count) = 0;

            virtual ~SequentialInputStream() = default;
        };

        class SequentialOutputStream
        {
        public:

            virtual void Write(const uint8_t * buffer, size_t count) = 0;

            virtual ~SequentialOutputStream() = default;
        };

        /*
        class SequentialIoStream : public SequentialInputStream, public SequentialOutputStream
        {
        };
        */
    }
}
