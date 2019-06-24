#pragma once

#include "Awl/Exception.h"
#include "Awl/StringFormat.h"

namespace awl::io
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

    class ReadFailException : public IoException
    {
        AWL_IMPLEMENT_EXCEPTION
    };

    class WriteFailException : public IoException
    {
        AWL_IMPLEMENT_EXCEPTION
    };

    //The exception indicating a general IO error in the user code.
    //When the user does an IO operation he throws IoError (or an exception of another type derived from IoException)
    //but catches IoException.
    class IoError : public IoException
    {
    private:

        const String theMessage;

    public:

        explicit IoError(String message) : theMessage(message)
        {
        }

        String GetMessage() const override
        {
            return theMessage;
        }

        AWL_IMPLEMENT_EXCEPTION
    };

    class FieldNotFoundException : public IoException
    {
    public:

        FieldNotFoundException(const std::string & name) : fieldName(name)
        {
        }

        String GetMessage() const override
        {
            return format() << _T("Field '") << FromAString(fieldName) << _T("' not found.") << _T(" .");
        }

        AWL_IMPLEMENT_EXCEPTION

    private:

        const std::string fieldName;
    };

    class TypeMismatchException : public IoException
    {
    public:

        TypeMismatchException(const std::string & name, size_t actual, size_t expected) :
            fieldName(name), actualType(actual), expectedType(expected)
        {
        }

        String GetMessage() const override
        {
            return format() << _T("Expected '") << FromAString(fieldName) << _T("' type: ") << expectedType << _T(" actually read type: ") << actualType << _T(" .");
        }

        AWL_IMPLEMENT_EXCEPTION

    private:

        const std::string fieldName;
        const size_t actualType;
        const size_t expectedType;
    };
}
