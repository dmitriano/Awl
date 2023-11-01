/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/String.h"

namespace awl
{
#ifdef AWL_QT

    class LogString
    {
    public:

        LogString(const char* m) : m_message(m) {}

        LogString(const wchar_t* m) : m_message(QString::fromWCharArray(m)) {}

        LogString(std::string message) : m_message(QString::fromStdString(message)) {}

        LogString(std::wstring message) : m_message(QString::fromStdWString(message)) {}

        LogString(QString message) : m_message(std::move(message)) {}

        const QString& str() const
        {
            return m_message;
        }

        QString& str()
        {
            return m_message;
        }

    private:

        // Make the Logger compatible with QDebug.
        QString m_message;
    };

#else

    class LogString
    {
    public:

        LogString(const char* m) : m_message(awl::FromACString(m)) {}

        LogString(const wchar_t* m) : m_message(awl::FromWCString(m)) {}

        LogString(std::string message) : m_message(awl::FromAString(message)) {}

        LogString(std::wstring message) : m_message(awl::FromWString(message)) {}

        String& str()
        {
            return m_message;
        }

        const String& str() const
        {
            return m_message;
        }

    private:

        awl::String m_message;
    };

#endif //AWL_QT
}
