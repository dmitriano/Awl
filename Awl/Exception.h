#pragma once

#include "Awl/String.h"

#include <exception>

namespace awl
{
    class Exception : public std::exception
    {
    public:

        virtual const Char * GetClassName() const
        {
            return _T("awl::Exception.");
        }

        virtual String GetMessage() const = 0;

        const char * what() const throw() override
        {
            return "awl exception";
        }
    };
}
