#pragma once

#include <string>
#include <sstream>

#ifdef _MSC_VER

#include <tchar.h>

#else

#ifndef TCHAR
#define TCHAR char
#endif

#if !defined(_T)	
#define _T(quoted_string) quoted_string
#endif

#endif

namespace awl
{
    typedef TCHAR Char;

    typedef std::basic_string<Char> String;

    typedef std::basic_istream<Char> istream;
    typedef std::basic_ostream<Char> ostream;
    
    typedef std::basic_ostringstream<Char> ostringstream;
}
