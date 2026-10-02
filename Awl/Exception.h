/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/String.h"

#include <exception>
#include <string>
#include <utility>

namespace awl
{
    class Exception : public std::exception
    {
    public:

        const char* what() const noexcept override = 0;

        virtual String message() const
        {
            return fromACString(what());
        }
    };

    class GeneralException : public Exception
    {
    protected:

        const std::string _message;

    public:

        explicit GeneralException(std::string message) :
            _message(std::move(message))
        {}

        explicit GeneralException(std::wstring message) :
            _message(encodeString(message.c_str()))
        {}

        const char* what() const noexcept override
        {
            return _message.c_str();
        }
    };
}

#define AWL_DEFINE_DERIVED_EXCEPTION(DerivedClass, BaseClass) \
    class DerivedClass : public BaseClass \
    { \
        public: \
        explicit DerivedClass(std::string message) : BaseClass(std::move(message)) {} \
        explicit DerivedClass(std::wstring message) : BaseClass(std::move(message)) {} \
    };

#define AWL_DEFINE_EXCEPTION(ExceptionClass) AWL_DEFINE_DERIVED_EXCEPTION(ExceptionClass, awl::GeneralException)
