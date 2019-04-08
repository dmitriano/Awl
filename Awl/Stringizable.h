#pragma once

#include "Awl/Serializable.h"

#include <vector>
#include <algorithm>
#include <assert.h>

namespace awl::helpers
{
    class MemberList
    {
    private:

        typedef std::vector<const char *> Vector;

    public:

        MemberList(char * s) : m_v(getArgNames(s))
        {
        }

        typedef Vector::const_reference const_reference;
        //typedef Vector::pointer pointer;
        typedef Vector::const_pointer const_pointer;
        //typedef Vector::iterator iterator;
        typedef Vector::const_iterator const_iterator;
        //typedef Vector::reverse_iterator reverse_iterator;
        typedef Vector::const_reverse_iterator const_reverse_iterator;

        const_iterator begin() const { return m_v.begin();}
        const_iterator cbegin() const noexcept { return m_v.cbegin();}

        const_iterator end() const { return m_v.end();}
        const_iterator cend() const noexcept { return m_v.cend();}

        const_iterator find(const char * name) const
        {
            return std::find_if(m_v.begin(), m_v.end(), [name](const char * p)
            {
                return strcmp(p, name) == 0;
            });
        }

        static inline const size_t NotAnIndex = static_cast<size_t>(-1);

        size_t find_index(const char * name)
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

        template <class Split, class Accept>
        static void traverse(char * va, Split split, Accept accept)
        {
            bool eaten_whitespace = true;

            char * p = va;
            while (*p != 0)
            {
                if (*p == ',')
                {
                    split(p);
                }
                else if (*p == ' ')
                {
                    split(p);
                    eaten_whitespace = true;
                }
                else
                {
                    if (eaten_whitespace)
                    {
                        accept(p);
                        eaten_whitespace = false;
                    }
                }

                ++p;
            }
        }
        
        static size_t getArgCount(char * va)
        {
            size_t count = 0;

            traverse(va, [](char *) {}, [&count](const char *) { ++count; });

            return count;
        }

        static Vector getArgNames(char * va)
        {
            Vector v;

            const size_t count = getArgCount(va);
            v.reserve(count);

            traverse(va, [](char * p) { *p = 0; }, [&v](const char * p) { v.push_back(p); });

            assert(v.size() == count);

            return v;
        }

        Vector m_v;
    };
}

#define AWL_STRINGIZABLE(...) \
    AWL_SERIALIZABLE(__VA_ARGS__) \
    static const awl::helpers::MemberList & get_member_list() \
    { \
        static std::string va = #__VA_ARGS__; \
        static const awl::helpers::MemberList ml(va.data()); \
        return ml; \
    }
