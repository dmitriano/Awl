#pragma once

#include "Awl/FunctionTraits.h"
#include "Awl/Serializable.h"

#include <type_traits>
#include <memory>

namespace awl
{
    template <class T, class GetKey>
    class KeyCompare
    {
    public:

        //The type GetKey applied to, for example 'class A { int key; std::string other; };'.
        using element_type = T;

        //The same as Container::value_type, for example std::shared_ptr<A>.
        using value_type = T;

        //The type of the key for heterogeneous lookup.
        using key_type = typename awl::function_traits<GetKey>::result_type;

        KeyCompare() = default;
        
        KeyCompare(GetKey && get_key) : getKey(std::forward<GetKey>(get_key))
        {
        }

        constexpr bool operator()(const T& left, const T& right) const
        {
            return getKey(left) < getKey(right);
        }

        constexpr bool operator()(const T& val, const key_type & id) const
        {
            return getKey(val) < id;
        }

        constexpr bool operator()(const key_type & id, const T& val) const
        {
            return id < getKey(val);
        }

    private:

        GetKey getKey;
    };

    template <class T, class GetKey>
    class KeyCompare<T *, GetKey>
    {
    public:

        using value_type = T *;
        using element_type = T;
        using key_type = typename awl::function_traits<GetKey>::result_type;

        KeyCompare() = default;

        KeyCompare(GetKey && get_key) : getKey(std::forward<GetKey>(get_key))
        {
        }

        constexpr bool operator()(const T * left, const T * right) const
        {
            return getKey(*left) < getKey(*right);
        }

        constexpr bool operator()(const T * val, const key_type & id) const
        {
            return getKey(*val) < id;
        }

        constexpr bool operator()(const key_type & id, const T * val) const
        {
            return id < getKey(*val);
        }

    private:

        GetKey getKey;
    };

    template <class T, class GetKey>
    class KeyCompare<std::shared_ptr<T>, GetKey>
    {
    public:

        using value_type = std::shared_ptr<T>;
        using element_type = T;
        using key_type = typename awl::function_traits<GetKey>::result_type;

        KeyCompare() = default;

        KeyCompare(GetKey && get_key) : getKey(std::forward<GetKey>(get_key))
        {
        }

        constexpr bool operator()(const std::shared_ptr<T> & left, const std::shared_ptr<T> & right) const
        {
            return getKey(*left) < getKey(*right);
        }

        constexpr bool operator()(const std::shared_ptr<T> & val, const key_type & id) const
        {
            return getKey(*val) < id;
        }

        constexpr bool operator()(const key_type & id, const std::shared_ptr<T> & val) const
        {
            return id < getKey(*val);
        }

    private:

        GetKey getKey;
    };

    template <class T, class Deleter, class GetKey>
    class KeyCompare<std::unique_ptr<T, Deleter>, GetKey>
    {
    public:

        using value_type = std::unique_ptr<T, Deleter>;
        using element_type = T;
        using key_type = typename awl::function_traits<GetKey>::result_type;

        KeyCompare() = default;

        KeyCompare(GetKey && get_key) : getKey(std::forward<GetKey>(get_key))
        {
        }

        constexpr bool operator()(const std::unique_ptr<T, Deleter> & left, const std::unique_ptr<T, Deleter> & right) const
        {
            return getKey(*left) < getKey(*right);
        }

        constexpr bool operator()(const std::unique_ptr<T, Deleter> & val, const key_type & id) const
        {
            return getKey(*val) < id;
        }

        constexpr bool operator()(const key_type & id, const std::unique_ptr<T, Deleter> & val) const
        {
            return id < getKey(*val);
        }

    private:

        GetKey getKey;
    };

    template <class T> struct remove_smart_pointer { typedef T type; };
    template <class T> struct remove_smart_pointer<T*> { typedef T type; };
    template <class T> struct remove_smart_pointer<T* const> { typedef T type; };
    template <class T> struct remove_smart_pointer<T* volatile> { typedef T type; };
    template <class T> struct remove_smart_pointer<T* const volatile> { typedef T type; };
    template <class T> struct remove_smart_pointer<std::shared_ptr<T>> { typedef T type; };
    template <class T, class Deleter> struct remove_smart_pointer<std::unique_ptr<T, Deleter>> { typedef T type; };

    template <class T>
    using remove_smart_pointer_t = typename remove_smart_pointer<T>::type;

    template <class T, class Field, Field T::*field_ptr>
    struct FieldGeter
    {
        const Field & operator() (const T & val) const
        {
            return val.*field_ptr;
        }
    };
    
    template <class T, class Field, Field remove_smart_pointer_t<T>::*field_ptr>
    using FieldCompare = KeyCompare<T, FieldGeter<remove_smart_pointer_t<T>, Field, field_ptr>>;

    template <class T, class Field, const Field & (T::*func_ptr)() const>
    struct LValueGeter
    {
        const Field & operator() (const T & val) const
        {
            return (val.*func_ptr)();
        }
    };

    template <class T, class Field, const Field & (remove_smart_pointer_t<T>::*func_ptr)() const>
    using LValueCompare = KeyCompare<T, LValueGeter<remove_smart_pointer_t<T>, Field, func_ptr>>;

    //A function that returns something like std::tie(x, y, z).
    template <class T, class Field, Field (T::*func_ptr)() const>
    struct RValueGeter
    {
        Field operator() (const T & val) const
        {
            return (val.*func_ptr)();
        }
    };

    template <class T, class Field, Field (remove_smart_pointer_t<T>::*func_ptr)() const>
    using RValueCompare = KeyCompare<T, RValueGeter<remove_smart_pointer_t<T>, Field, func_ptr>>;

    template <class T, size_t index>
    struct TuplizableGeter
    {
        const auto & operator() (const T & val) const
        {
            return std::get<index>(val.as_const_tuple());
        }
    };

    template <class T, size_t index>
    using TuplizableCompare = KeyCompare<T, TuplizableGeter<remove_smart_pointer_t<T>, index>>;
}
