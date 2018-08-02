#pragma once

#include <string>

#ifndef _MSC_VER

#ifndef TCHAR
#define TCHAR char
#endif

#if !defined(_T)	
#define _T(quoted_string) quoted_string
#endif

#else

#include <tchar.h>

#endif

namespace awl
{
    typedef TCHAR Char;

    typedef std::basic_string<Char> String;
}
