/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/String.h"

#include <exception>
#include <string>
#include <typeinfo>
#include <utility>

namespace awl
{
    class Exception : public std::exception
    {
    public:

        String className() const
        {
            return fromACString(what());
        }
        
        virtual String message() const
        {
            return className();
        }

        const char * what() const noexcept override
        {
            #if !defined(AWL_NO_RTTI)
                return typeid(*this).name();
            #else
                return "AWL Exception";
            #endif
        }
    };

    class GeneralException : public Exception
    {
    protected:

        const String m_message;

    public:

        explicit GeneralException(std::string message) :
            m_message(fromAString(std::move(message)))
        {}

        explicit GeneralException(std::wstring message) :
            m_message(fromWString(std::move(message)))
        {}

        String message() const override
        {
            return m_message;
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
