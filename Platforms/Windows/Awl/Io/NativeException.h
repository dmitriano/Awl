/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Exception.h"
#include "Awl/StringFormat.h"

#include "Awl/Io/Platform.h"

namespace awl::io
{
    class Win32Exception : public Exception
    {
    public:

        Win32Exception(DWORD error = ::GetLastError()) : dwError(error)
        {
        }

        String What() const override
        {
            return format() << _T("Fatal Win32 exception. Error code: ") << dwError << _T(" .");
        }

    private:

        const DWORD dwError;
    };

    using NativeException = Win32Exception;
}
