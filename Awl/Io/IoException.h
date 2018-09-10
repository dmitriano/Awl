#pragma once

#include "Awl/Exception.h"
#include "Awl/StringFormat.h"

namespace awl
{
    namespace io
    {
        class IoException : public Exception
        {
            AWL_IMPLEMENT_EXCEPTION
        };

        class EndOfFileException : public IoException
        {
        public:

            EndOfFileException(size_t requested_count, size_t actually_read_count) :
                requestedCount(requested_count), actuallyReadCount(actually_read_count)
            {
            }

            String GetMessage() const override
            {
                return format() << _T("Requested ") << requestedCount << _T(" actually read ") << actuallyReadCount << _T(" .");
            }

            AWL_IMPLEMENT_EXCEPTION

        private:

            const size_t requestedCount;
            const size_t actuallyReadCount;
        };

        class CorruptionException : public IoException
        {
        public:

            CorruptionException(size_t pos = -1) : m_pos(pos)
            {
            }

            String GetMessage() const override
            {
                format out;
                
                out << _T("The stream is corrupted");

                if (m_pos != static_cast<size_t>(-1))
                {
                    out << _T(" at ") << m_pos;
                }

                out << _T(" .");

                return out;
            }

            AWL_IMPLEMENT_EXCEPTION

        private:

            const size_t m_pos;
        };
    }
}
