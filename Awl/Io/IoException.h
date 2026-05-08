/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Exception.h"
#include "Awl/StringFormat.h"

namespace awl::io
{
    class IoException : public Exception {};

    class EndOfFileException : public IoException
    {
    public:

        EndOfFileException(size_t requested_count, size_t actually_read_count) :
            requestedCount(requested_count), actuallyReadCount(actually_read_count)
        {
        }

        String message() const override
        {
            return std::format(_T("Requested {} actually read {} ."), requestedCount, actuallyReadCount);
        }

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

        String message() const override
        {
            awl::ostringstream out;

            out << _T("The stream is corrupted");

            if (m_pos != static_cast<size_t>(-1))
            {
                out << _T(" at ") << m_pos;
            }

            out << _T(" .");

            return out.str();
        }

    private:

        const size_t m_pos;
    };

    class ReadFailException : public IoException {};

    class WriteFailException : public IoException {};

    //The exception indicating a general IO error in the user code.
    //When the user does an IO operation he throws IoError (or an exception of another type derived from IoException)
    //but catches IoException.
    class IoError : public IoException
    {
    private:

        const String theMessage;

    public:

        explicit IoError(String message) : theMessage(std::move(message))
        {
        }

        String message() const override
        {
            return theMessage;
        }
    };

    class FieldNotFoundException : public IoException
    {
    public:

        FieldNotFoundException(std::string name) : fieldName(name)
        {
        }

        String message() const override
        {
            return std::format(_T("Field '{}' not found. ."), fromAString(fieldName));
        }

    private:

        const std::string fieldName;
    };

    class TypeMismatchException : public IoException
    {
    public:

        TypeMismatchException(std::string name, size_t actual, size_t expected) :
            fieldName(name), actualType(actual), expectedType(expected)
        {
        }

        String message() const override
        {
            return std::format(_T("Expected '{}' type: {} actually read type: {} ."), fromAString(fieldName), expectedType, actualType);
        }

    private:

        const std::string fieldName;
        const size_t actualType;
        const size_t expectedType;
    };
}
