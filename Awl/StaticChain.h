/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/SingleList.h"
#include "Awl/String.h"
#include "Awl/StaticSingleton.h"

#include <ranges>

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

        Iterator find_iter(const char* name)
        {
            ConstIterator i = const_cast<const StaticChain*>(this)->find_iter(name);

            const Link* p_link = *i;

            return Iterator(const_cast<Link*>(p_link));
        }

        ConstIterator find_iter(const char* name) const
        {
            return std::find_if(begin(), end(),
                [name](const Link* link) -> bool
                {
                    return StrCmp(link->name(), name) == 0;
                });
        }

        Iterator find_iter(const std::string& name)
        {
            return find_iter(name.c_str());
        }

        ConstIterator find_iter(const std::string& name) const
        {
            return find_iter(name.c_str());
        }

        template <class Pred = CStringInsensitiveEqual<char>>
        Link* find_ptr(const char* name, Pred&& pred = {})
        {
            return const_cast<Link*>((const_cast<const StaticChain*>(this))->find_ptr(name, pred));
        }

        template <class Pred = CStringInsensitiveEqual<char>>
        const Link* find_ptr(const char* name, Pred&& pred = {}) const
        {
            // list can't do this.
            //auto i = std::ranges::find_if(begin(), end(),
            //    std::bind(pred, name, std::placeholders::_1),
            //    std::mem_fn(&Link::name));

            auto i = std::find_if(begin(), end(),
                [&pred, name](const Link* link) -> bool
                {
                    return StrCmp(link->name(), name) == 0;
                });

            if (i != end())
            {
                return *i;
            }

            return nullptr;
        }

        template <class Pred = CStringInsensitiveEqual<char>>
        Link* find_ptr(const std::string& name, Pred&& pred = {})
        {
            return find_ptr(name.c_str(), pred);
        }

        template <class Pred = CStringInsensitiveEqual<char>>
        const Link* find_ptr(const std::string& name, Pred&& pred = {}) const
        {
            return find_ptr(name.c_str(), pred);
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

    // It is not clear what can be the usage of variable_static_chain()
    // in the real-life code, but I left it here just in case.
    // Declaring a StaticLink as const and then using variable_static_chain()
    // means accessing a const value via non-const pointer,
    // see StaticLink constructor.
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
        // We circumvent const-correctness here.
        // Declaring StaticChain as const can potentially lead to UB.
        // Somewhere in the code StaticChain can be accessed as the list element and changed.
        // 
        // From C++ standard:
        // const and volatile semantics (7.1.6.1) are not applied on an object under construction.
        // They come into effect when the constructor for the most derived object (1.8) ends.

        variable_static_chain<T>().m_list.push_front(this);
    }
}
