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

        virtual String GetMessage() const = 0;

        AWL_IMPLEMENT_EXCEPTION
    };
}
