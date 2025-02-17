/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/String.h"
#include "Awl/StaticChain.h"
#include "Awl/StringFormat.h"
#include "Awl/KeyCompare.h"

#include <vector>
#include <algorithm>
#include <stdexcept>
#include <regex>
#include <ranges>

namespace awl
{
    template <class T>
    class StaticMap
    {
    private:

        using LinkVector = std::vector<const StaticLink<T>*>;

    public:

        typename LinkVector::const_iterator begin() const
        {
            return m_links.begin();
        }

        typename LinkVector::const_iterator end() const
        {
            return m_links.end();
        }

        typename std::size_t size() const
        {
            return m_links.size();
        }

        const StaticLink<T>* find(const char* name) const
        {
            return internal_find(m_links, name);
        }

        template <class Pred = true_predicate<T>>
        static StaticMap fill(const std::string& name_filter = {}, Pred&& value_filter = {})
        {
            LinkVector links;

            auto insert_links = [&links, &value_filter](auto name_pred)
            {
                for (const StaticLink<T>* p_link : awl::static_chain<T>())
                {
                    if (value_filter(p_link->value()) && name_pred(p_link->name()))
                    {
                        if (internal_find(links, p_link->name()) != nullptr)
                        {
                            throw std::runtime_error(aformat() << "Static link '" << p_link->name() << " already exists.");
                        }

                        links.push_back(p_link);
                    }
                }
            };

            if (name_filter.empty())
            {
                insert_links([](const char*) -> bool { return true; });
            }
            else
            {
                std::basic_regex<char> name_regex(name_filter);

                insert_links([&name_regex](const char* name) -> bool
                {
                    std::match_results<const char*> match;

                    return std::regex_match(name, match, name_regex);
                });
            }

            std::sort(links.begin(), links.end(), LinkCompare());

            return StaticMap<T>(links);
        }

    private:

        static const StaticLink<T>* internal_find(const LinkVector& links, const char* name)
        {
            auto i = std::lower_bound(links.begin(), links.end(), name, LinkCompare());

            StringInsensitiveEqual<char> equal;

            if (i == links.end() || !equal((*i)->name(), name))
            {
                return nullptr;
            }

            return *i;
        }

        explicit StaticMap(LinkVector links) : m_links(std::move(links)) {}

        using LinkCompare = awl::pointer_compare<&StaticLink<T>::name, awl::CStringInsensitiveLess<char>>;

        const LinkVector m_links;
    };
}
