/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Tuplizable.h"

#include <vector>
#include <algorithm>
#include <assert.h>
#include <string>
#include <cstring>

namespace awl
{
    namespace helpers
    {
        class MemberList
        {
        private:

            using Vector = std::vector<std::string>;

        public:

            MemberList(const char * s) : m_v(getArgNames(s))
            {
            }

            using const_reference = Vector::const_reference;
            using const_pointer = Vector::const_pointer;
            using const_iterator = Vector::const_iterator;
            using const_reverse_iterator = Vector::const_reverse_iterator;

            const_iterator begin() const { return m_v.begin();}
            const_iterator cbegin() const noexcept { return m_v.cbegin();}

            const_iterator end() const { return m_v.end();}
            const_iterator cend() const noexcept { return m_v.cend();}

            const_iterator find(const std::string & name) const
            {
                return std::find(m_v.begin(), m_v.end(), name);
            }

            const_iterator find_cstr(const char * name) const
            {
                return std::find_if(m_v.begin(), m_v.end(), [name](const std::string & val)
                {
                    return std::strcmp(val.c_str(), name) == 0;
                });
            }

            static inline const size_t NotAnIndex = static_cast<size_t>(-1);

            size_t find_index(const std::string & name)
            {
                const_iterator i = find(name);

                if (i != m_v.end())
                {
                    return i - m_v.begin();
                }

                return NotAnIndex;
            }

            const_reference operator[](size_t pos) const
            {
                return m_v[pos];
            }

            size_t size() const
            {
                return m_v.size();
            }

        private:

            template <class Accept>
            static void traverse(const char * va, Accept accept)
            {
                const char * p_start = va;
                bool eaten_separator = false;

                const char * p = va;
                while (*p != 0)
                {
                    if (*p == ',' && *(p + 1) == ' ')
                    {
                        assert(p_start != nullptr);
                        assert(p > p_start);
                        accept(p_start, p - p_start);
                        p_start = nullptr;
                        eaten_separator = true;
                        ++p;
                    }
                    else if (eaten_separator)
                    {
                        p_start = p;
                        eaten_separator = false;
                    }

                    ++p;
                }

                assert(p_start != nullptr);

                //the list can be empty
                if (p != p_start)
                {
                    assert(p > p_start);
                    accept(p_start, p - p_start);
                }
            }

            static size_t getArgCount(const char * va)
            {
                size_t count = 0;

                traverse(va, [&count](const char *, size_t) { ++count; });

                return count;
            }

            static Vector getArgNames(const char * va)
            {
                Vector v;

                const size_t count = getArgCount(va);
                v.reserve(count);

                traverse(va, [&v](const char * p, size_t len)
                {
                    v.push_back(std::string(p, len)); }
                );

                assert(v.size() == count);

                return v;
            }

            Vector m_v;
        };
    }

    template <typename T, typename = void>
    struct is_stringizable_impl : std::false_type {};

    template <typename T>
    struct is_stringizable_impl<T, std::void_t<decltype(T{}.get_member_names())>> : std::true_type {};

    // Hide an extra template parameter.
    template <typename T>
    struct is_stringizable : is_stringizable_impl<T> {};

    template <typename T>
    inline constexpr bool is_stringizable_v = is_stringizable<T>::value;

    struct EmptyStringizable
    {
        constexpr auto as_const_tuple() const
        {
            return std::tie();
        }
        constexpr auto as_tuple() const
        {
            return as_const_tuple();
        }
        constexpr auto as_tuple()
        {
            return std::tie();
        }

        static const awl::helpers::MemberList& get_member_names()
        {
            static const awl::helpers::MemberList ml("");
            return ml;
        }
    };
}

#define AWL_STRINGIZABLE(...) \
    AWL_TUPLIZABLE(__VA_ARGS__) \
    static const awl::helpers::MemberList & get_member_names() \
    { \
        static const char va[] = #__VA_ARGS__; \
        static const awl::helpers::MemberList ml(va); \
        return ml; \
    }

