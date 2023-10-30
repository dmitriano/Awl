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
    class Win32Exception : public IoError
    {
    public:

        Win32Exception(DWORD error = ::GetLastError()) :
            Win32Exception(_T("A Win32 API call failed."), error)
        {
        }

        Win32Exception(String message, DWORD error = ::GetLastError()) :
            IoError(std::move(message)),
            dwError(error)
        {
        }

        String What() const override
        {
            return format() << IoError::What() << _T(" Error code: ") << dwError << _T(" .");
        }

    private:

        const DWORD dwError;
    };

    using NativeException = Win32Exception;
}
