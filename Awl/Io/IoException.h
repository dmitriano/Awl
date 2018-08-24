#pragma once

#include "Awl/Exception.h"
#include "Awl/StringFormat.h"

namespace awl
{
    namespace io
    {
        class IoException : public Exception
        {
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
                return format() << _T("Requested ") << requestedCount << _T(" actually read ") << actuallyReadCount << _T(".");
            }

        private:

            const size_t requestedCount;
            const size_t actuallyReadCount;
        };
    }
}
