#pragma once

#include "Awl/EnumTraits.h"

#include <array>
#include <cstddef>

namespace awl
{
    template <is_sequential_enum Enum, class T>
    class enum_array
    {
    public:
        using enum_type = Enum;
        using array_type = std::array<T, EnumTraits<Enum>::count()>;
        using value_type = typename array_type::value_type;
        using size_type = typename array_type::size_type;
        using difference_type = typename array_type::difference_type;
        using reference = typename array_type::reference;
        using const_reference = typename array_type::const_reference;
        using pointer = typename array_type::pointer;
        using const_pointer = typename array_type::const_pointer;
        using iterator = typename array_type::iterator;
        using const_iterator = typename array_type::const_iterator;
        using reverse_iterator = typename array_type::reverse_iterator;
        using const_reverse_iterator = typename array_type::const_reverse_iterator;

        reference at(enum_type index)
        {
            return m_items.at(enum_to_index(index));
        }

        const_reference at(enum_type index) const
        {
            return m_items.at(enum_to_index(index));
        }

        reference at(size_type index)
        {
            return m_items.at(index);
        }

        const_reference at(size_type index) const
        {
            return m_items.at(index);
        }

        reference operator[](enum_type index)
        {
            return m_items[enum_to_index(index)];
        }

        const_reference operator[](enum_type index) const
        {
            return m_items[enum_to_index(index)];
        }

        reference operator[](size_type index)
        {
            return m_items[index];
        }

        const_reference operator[](size_type index) const
        {
            return m_items[index];
        }

        reference front()
        {
            return m_items.front();
        }

        const_reference front() const
        {
            return m_items.front();
        }

        reference back()
        {
            return m_items.back();
        }

        const_reference back() const
        {
            return m_items.back();
        }

        pointer data() noexcept
        {
            return m_items.data();
        }

        const_pointer data() const noexcept
        {
            return m_items.data();
        }

        iterator begin()
        {
            return m_items.begin();
        }

        const_iterator begin() const
        {
            return m_items.begin();
        }

        const_iterator cbegin() const
        {
            return m_items.cbegin();
        }

        iterator end()
        {
            return m_items.end();
        }

        const_iterator end() const
        {
            return m_items.end();
        }

        const_iterator cend() const
        {
            return m_items.cend();
        }

        reverse_iterator rbegin()
        {
            return m_items.rbegin();
        }

        const_reverse_iterator rbegin() const
        {
            return m_items.rbegin();
        }

        const_reverse_iterator crbegin() const
        {
            return m_items.crbegin();
        }

        reverse_iterator rend()
        {
            return m_items.rend();
        }

        const_reverse_iterator rend() const
        {
            return m_items.rend();
        }

        const_reverse_iterator crend() const
        {
            return m_items.crend();
        }

        bool empty() const
        {
            return m_items.empty();
        }

        size_type size() const
        {
            return m_items.size();
        }

        size_type max_size() const
        {
            return m_items.max_size();
        }

        void fill(const value_type& value)
        {
            m_items.fill(value);
        }

        void swap(enum_array& other) noexcept(noexcept(m_items.swap(other.m_items)))
        {
            m_items.swap(other.m_items);
        }

    private:
        array_type m_items = {};
    };
}
