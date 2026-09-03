/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Crypto/BasicHash.h"
#include "Awl/String.h"

#include <iostream>
#include <iomanip>
#include <chrono>
#include <cstddef>

namespace awl::testing::helpers
{
    template <size_t N>
    awl::ostream & operator << (awl::ostream & out, awl::crypto::HashValue<N> & h)
    {
        out << _T("0x");

        for (size_t i = 0; i < N; ++i)
        {
            out << std::hex << std::setfill(_T('0')) << std::setw(2) << std::to_integer<unsigned int>(h[i]) << std::dec;
        }

        return out;
    }

    awl::ostream & operator << (awl::ostream & out, std::chrono::steady_clock::duration d);
}
