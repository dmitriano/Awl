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

        String What() const override
        {
            return format() << IoError::What() << _T(" Error code: ") << m_error << _T(" .");
        }

    private:

        const int m_error;
    };

    using NativeException = PosixException;
}
