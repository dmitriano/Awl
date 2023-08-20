#pragma once

#include <string_view>

#if defined(__APPLE__) || defined(__ANDROID__)

    namespace awl
    {
        template<class C, class It, class End >
        constexpr std::basic_string_view<C> make_string_view(It begin, End end)
        {
            const C* s = std::addressof(*begin);
            typename std::basic_string_view<C>::size_type count = static_cast<typename std::basic_string_view<C>::size_type>(end - begin);

            return std::basic_string_view<C>(s, count);
        }
    }

#else

    namespace awl
    {
        template<class C, class It, class End >
        constexpr std::basic_string_view<C> make_string_view(It begin, End end)
        {
            return std::basic_string_view<C>(begin, end);
        }
    }

#endif
