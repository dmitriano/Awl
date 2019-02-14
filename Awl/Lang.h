#pragma once

#if AWL_CPPSTD >= 14
    #define AWL_CONSTEXPR constexpr
#else
    #define AWL_CONSTEXPR
#endif
