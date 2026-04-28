/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/IoException.h"
#include "Awl/StringFormat.h"

#include "Awl/Io/Platform.h"

namespace awl::io
{
    class PosixException : public IoError
    {
    public:

        using IoError::IoError;

        PosixException(int error = errno) : 
            PosixException("Posix exception.", error)
        {}

        PosixException(String message, int error = errno) : 
            IoError(std::move(message)),
            m_error(error)
        {}

        String message() const override
        {
            return std::format(_T("{} Error code: {} ."), IoError::message(), m_error);
        }

    private:

        const int m_error;
    };

    using NativeException = PosixException;
}
