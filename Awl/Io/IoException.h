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
        {}

        const char* what() const noexcept override
        {
            return "End of file.";
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

        CorruptionException(size_t pos = -1) : _pos(pos)
        {}

        const char* what() const noexcept override
        {
            return "The stream is corrupted.";
        }

        String message() const override
        {
            awl::ostringstream out;

            out << _T("The stream is corrupted");

            if (_pos != static_cast<size_t>(-1))
            {
                out << _T(" at ") << _pos;
            }

            out << _T(" .");

            return out.str();
        }

    private:

        const size_t _pos;
    };

    class ReadFailException : public IoException
    {
    public:

        const char* what() const noexcept override
        {
            return "Read failed.";
        }
    };

    class WriteFailException : public IoException
    {
    public:

        const char* what() const noexcept override
        {
            return "Write failed.";
        }
    };

    //The exception indicating a general IO error in the user code.
    //When the user does an IO operation he throws IoError (or an exception of another type derived from IoException)
    //but catches IoException.
    class IoError : public IoException
    {
    private:

        const std::string theMessage;

    public:

        explicit IoError(String message) : theMessage(toAString(std::move(message)))
        {}

        const char* what() const noexcept override
        {
            return theMessage.c_str();
        }
    };

    class FieldNotFoundException : public IoException
    {
    public:

        FieldNotFoundException(std::string name) : fieldName(name)
        {}

        const char* what() const noexcept override
        {
            return "Field not found.";
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
        {}

        const char* what() const noexcept override
        {
            return "Type mismatch.";
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
