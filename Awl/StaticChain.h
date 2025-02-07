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
        StaticLink(const char* p_name, Args&&... args);

        const char* name() const
        {
            return pName;
        }

        const T& value() const
        {
            return m_val;
        }

        // Use variable_static_chain() to access non-const value,
        T& value()
        {
            return m_val;
        }

    private:

        const char* const pName;

        T m_val;
    };

    template <class T>
    class StaticChain
    {
    private:

        using Link = StaticLink<T>;
        using List = single_list<Link>;
        using Iterator = typename List::iterator;
        using ConstIterator = typename List::const_iterator;

    public:

        ConstIterator begin() const { return m_list.begin(); }
        ConstIterator end() const { return m_list.end(); }

        Iterator begin() { return m_list.begin(); }
        Iterator end() { return m_list.end(); }

        Iterator find(const char* name)
        {
            return std::find_if(begin(), end(),
                [name](const Link* link) -> bool
                {
                    return StrCmp(link->name(), name) == 0;
                });
        }

        ConstIterator find(const char* name) const
        {
            return const_cast<StaticChain*>(this)->find(name);
        }

        Iterator find(const std::string& name)
        {
            return find(name.c_str());
        }

        ConstIterator find(const std::string& name) const
        {
            return const_cast<StaticChain*>(this)->find(name);
        }

        void clear()
        {
            while (!m_list.empty())
            {
                m_list.pop_front();
            }
        }

    private:

        template <class T1>
        friend class StaticLink;

        List m_list;
    };

    template <class T>
    StaticChain<T>& variable_static_chain()
    {
        return static_singleton<StaticChain<T>>();
    }

    template <class T>
    const StaticChain<T>& static_chain()
    {
        return variable_static_chain<T>();
    }

    template <class T>
    template <typename... Args>
    StaticLink<T>::StaticLink(const char* p_name, Args&&... args):
        pName(p_name),
        m_val(std::forward<Args>(args)...)
    {
        // Access non-const singleton.
        variable_static_chain<T>().m_list.push_front(this);
    }
}
