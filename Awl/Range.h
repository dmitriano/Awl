/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

namespace awl
{
    template <class Iterator>
    class range
    {
    public:

        range(Iterator begin, Iterator end) : m_begin(begin), m_end(end)
        {
        }

        Iterator begin() const
        {
            return m_begin;
        }

        Iterator end() const
        {
            return m_end;
        }

        bool empty() const
        {
            return begin() == end();
        }

    private:
        
        Iterator m_begin;

        Iterator m_end;
    };

    template <class Iterator>
    range<Iterator> make_range(Iterator begin, Iterator end)
    {
        return range<Iterator>(begin, end);
    }

    template <class Container>
    range<typename Container::iterator> make_range(Container & container)
    {
        return make_range(container.begin(), container.end());
    }

    template <class Container>
    range<typename Container::const_iterator> make_crange(const Container & container)
    {
        return make_range(container.begin(), container.end());
    }

    template <class Container>
    range<typename Container::const_iterator> make_range(const Container & container)
    {
        return make_crange(container);
    }
}
