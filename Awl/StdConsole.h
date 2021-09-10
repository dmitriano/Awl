/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/String.h"

#include <iostream>

namespace awl
{
    //We use std::cout/std::cin or std::wcout/std::wcin depending on Char typedef.
    template<typename T> struct select_console;

    template<>
    struct select_console<char>
    {
        static std::istream &cin() { return std::cin; }
        static std::ostream &cout() { return std::cout; }
    };

    template<>
    struct select_console<wchar_t>
    {
        static std::wistream &cin() { return std::wcin; }
        static std::wostream &cout() { return std::wcout; }
    };

    inline istream &cin() { return select_console<Char>::cin(); }
    inline ostream &cout() { return select_console<Char>::cout(); }
}
