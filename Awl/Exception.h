#pragma once

#include "Awl/String.h"

#include <exception>

#define AWL_IMPLEMENT_EXCEPTION \
    const char * what() const throw() override \
    { \
        return typeid(*this).name(); \
    }

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

        AWL_IMPLEMENT_EXCEPTION
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

        AWL_IMPLEMENT_EXCEPTION
    };
}

#define AWL_DEFINE_DERIVED_EXCEPTION(DerivedClass, BaseClass) \
    class DerivedClass : public BaseClass \
    { \
        public: \
        DerivedClass(awl::String message) : BaseClass(message) {} \
        AWL_IMPLEMENT_EXCEPTION \
    };

#define AWL_DEFINE_EXCEPTION(ExceptionClass) AWL_DEFINE_DERIVED_EXCEPTION(ExceptionClass, awl::GeneralException)
