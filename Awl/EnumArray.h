#pragma once

#include "Awl/EnumTraits.h"

#include <array>
#include <concepts>
#include <cstddef>
#include <tuple>
#include <utility>
namespace awl
{
    template <sequential_enum Enum, class T>
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

        enum_array() = default;

        enum_array(const enum_array&) = default;

        enum_array(enum_array&&) = default;

        enum_array& operator=(const enum_array&) = default;

        enum_array& operator=(enum_array&&) = default;

        enum_array(const array_type& items) :
            _items(items)
        {}

        enum_array(array_type&& items) :
            _items(std::move(items))
        {}

        template <class... Args>
            requires (sizeof...(Args) > 0 && sizeof...(Args) <= EnumTraits<Enum>::count() &&
                (std::constructible_from<value_type, Args&&> && ...))
        enum_array(Args&&... args) :
            _items{ std::forward<Args>(args)... }
        {}

        template <class... Args>
            requires (sizeof...(Args) > 0 && sizeof...(Args) <= EnumTraits<Enum>::count())
        enum_array(std::in_place_t, Args&&... args) :
            _items{ std::make_from_tuple<value_type>(std::forward<Args>(args))... }
        {}

        reference at(enum_type index)
        {
            return _items.at(enum_to_index(index));
        }

        const_reference at(enum_type index) const
        {
            return _items.at(enum_to_index(index));
        }

        reference at(size_type index)
        {
            return _items.at(index);
        }

        const_reference at(size_type index) const
        {
            return _items.at(index);
        }

        reference operator[](enum_type index)
        {
            return _items[enum_to_index(index)];
        }

        const_reference operator[](enum_type index) const
        {
            return _items[enum_to_index(index)];
        }

        reference operator[](size_type index)
        {
            return _items[index];
        }

        const_reference operator[](size_type index) const
        {
            return _items[index];
        }

        reference front()
        {
            return _items.front();
        }

        const_reference front() const
        {
            return _items.front();
        }

        reference back()
        {
            return _items.back();
        }

        const_reference back() const
        {
            return _items.back();
        }

        pointer data() noexcept
        {
            return _items.data();
        }

        const_pointer data() const noexcept
        {
            return _items.data();
        }

        iterator begin()
        {
            return _items.begin();
        }

        const_iterator begin() const
        {
            return _items.begin();
        }

        const_iterator cbegin() const
        {
            return _items.cbegin();
        }

        iterator end()
        {
            return _items.end();
        }

        const_iterator end() const
        {
            return _items.end();
        }

        const_iterator cend() const
        {
            return _items.cend();
        }

        reverse_iterator rbegin()
        {
            return _items.rbegin();
        }

        const_reverse_iterator rbegin() const
        {
            return _items.rbegin();
        }

        const_reverse_iterator crbegin() const
        {
            return _items.crbegin();
        }

        reverse_iterator rend()
        {
            return _items.rend();
        }

        const_reverse_iterator rend() const
        {
            return _items.rend();
        }

        const_reverse_iterator crend() const
        {
            return _items.crend();
        }

        bool empty() const
        {
            return _items.empty();
        }

        size_type size() const
        {
            return _items.size();
        }

        size_type max_size() const
        {
            return _items.max_size();
        }

        void fill(const value_type& value)
        {
            _items.fill(value);
        }

        void swap(enum_array& other) noexcept(noexcept(_items.swap(other._items)))
        {
            _items.swap(other._items);
        }

    private:

        array_type _items = {};
    };
}
