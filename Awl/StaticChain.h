/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/SingleList.h"
#include "Awl/String.h"
#include "Awl/StaticSingleton.h"

#include <algorithm>

namespace awl
{
    template <class T>
    class StaticLink : public single_link
    {
    public:

        template <typename... Args>
        StaticLink(const Char* p_name, Args&&... args);

        const Char* name() const
        {
            return pName;
        }

        T& value() const
        {
            return m_val;
        }

    private:

        const Char* const pName;

        // TODO: Get rid of mutable here.
        mutable T m_val;
    };

    template <class T>
    class StaticChain
    {
    private:

        using Link = StaticLink<T>;
        using List = single_list<Link>;
        using Iterator = typename List::const_iterator;

    public:

        Iterator begin() const { return m_list.begin(); }
        Iterator end() const { return m_list.end(); }

        Iterator find(const Char* name) const
        {
            return std::find_if(begin(), end(),
                [name](const Link* link) -> bool
                {
                    return StrCmp(link->name(), name) == 0;
                });
        }

        Iterator find(const String& name) const { return find(name.c_str()); }

    private:

        template <class T1>
        friend class StaticLink;

        List m_list;
    };

    template <class T>
    const StaticChain<T>& static_chain()
    {
        return static_singleton<StaticChain<T>>();
    }

    template <class T>
    template <typename... Args>
    StaticLink<T>::StaticLink(const Char* p_name, Args&&... args):
        pName(p_name),
        m_val(std::forward<Args>(args)...)
    {
        // Access non-const singleton.
        static_singleton<StaticChain<T>>().m_list.push_front(this);
    }
}
