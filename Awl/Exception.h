#pragma once

#include "Awl/String.h"

#include <exception>
#include <typeinfo>

namespace awl
{
    class Exception : public std::exception
    {
    public:

        String GetClassName() const
        {
            return FromACString(what());
        }
        
        virtual String GetMessage() const
        {
            return GetClassName();
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
    private:

        const String theMessage;

    public:

        explicit GeneralException(String message) : theMessage(message)
        {
        }

        String GetMessage() const override
        {
            return theMessage;
        }
    };
}

#define AWL_DEFINE_DERIVED_EXCEPTION(DerivedClass, BaseClass) \
    class DerivedClass : public BaseClass \
    { \
        public: \
        DerivedClass(awl::String message) : BaseClass(message) {} \
    };

#define AWL_DEFINE_EXCEPTION(ExceptionClass) AWL_DEFINE_DERIVED_EXCEPTION(ExceptionClass, awl::GeneralException)
