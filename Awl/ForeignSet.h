/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/ObservableSet.h"
#include "Awl/KeyCompare.h"
#include "Awl/TypeTraits.h"

#include <cassert>
#include <functional>

namespace awl
{
    template <class T, auto primary_key, auto foreign_key>
    class foreign_set : public Observer<INotifySetChanged<T>>
    {
    private:

        //Plain pointer and std::shared_ptr<A> -> themselves
        //std::unique_ptr<A> -> A *
        //another type A -> A *
        using Pointer = std::conditional_t<is_copyable_pointer_v<T>, T, const remove_pointer_t<T>*>;

        using ForeignKey = std::invoke_result_t<decltype(foreign_key), const remove_pointer_t<T>&>;
        using PrimaryCompare = KeyCompare<Pointer, primary_key>;
        using ValueSet = observable_vector_set<Pointer, PrimaryCompare>;

        class ValueSetCompare
        {
        public:

            ValueSetCompare() = default;

            constexpr bool operator()(const ValueSet & left, const ValueSet & right) const
            {
                return foreignKey(left) < foreignKey(right);
            }

            constexpr bool operator()(const ValueSet& val, const ForeignKey & id) const
            {
                return foreignKey(val) < id;
            }

            constexpr bool operator()(const ForeignKey & id, const ValueSet& val) const
            {
                return id < foreignKey(val);
            }

        private:

            constexpr ForeignKey foreignKey(const ValueSet & vs) const
            {
                assert(!vs.empty());
                return std::invoke(foreign_key, *vs.front());
            }
        };
        
        using MultiSet = observable_vector_set<ValueSet, ValueSetCompare>;
        using MultiSetObserver = Observer<INotifySetChanged<ValueSet>>;

    public:

        foreign_set() :
            _set()
        {}

        //TODO: Make it deduce template arguments.
        template <class SrcSet>
        foreign_set(const SrcSet& src_set) :
            foreign_set()
        {
            for (auto& val : src_set)
            {
                onAdded(val);
            }

            src_set.subscribe(this);
        }

        using value_type = ValueSet;

        using size_type = typename MultiSet::size_type;
        using difference_type = typename MultiSet::difference_type;
        using const_reference = typename MultiSet::const_reference;

        using const_iterator = typename MultiSet::const_iterator;
        using const_reverse_iterator = typename MultiSet::const_reverse_iterator;

        using allocator_type = typename MultiSet::allocator_type;
        using key_compare = typename MultiSet::key_compare;
        using value_compare = typename MultiSet::value_compare;

        const ValueSet & front() const { return _set.front(); }
        const ValueSet & back() const { return _set.back(); }

        const_iterator begin() const { return _set.begin(); }
        const_iterator end() const { return _set.end(); }
        const_reverse_iterator rbegin() const { return _set.rbegin(); }
        const_reverse_iterator rend() const { return _set.rend(); }

        bool empty() const
        {
            return _set.empty();
        }

        size_type size() const
        {
            return _set.size();
        }

        const_reference operator[](size_type pos) const
        {
            return _set[pos];
        }

        const_reference at(size_type pos) const
        {
            return _set.at(pos);
        }

        template <class Key>
        size_type index_of(const Key & key) const
        {
            return _set.index_of(key);
        }

        template <class Key>
        const_iterator find(const Key & key) const
        {
            return _set.find(key);
        }

        void subscribe(MultiSetObserver* p_observer) const
        {
            _set.subscribe(p_observer);
        }

        void unsubscribe(MultiSetObserver* p_observer) const
        {
            _set.unsubscribe(p_observer);
        }

    private:

        static constexpr Pointer valueToPointer(const T& val)
        {
            if constexpr (is_copyable_pointer_v<T>)
            {
                //T
                return val;
            }
            else if constexpr (is_specialization_v<T, std::unique_ptr>)
            {
                //const remove_pointer_t<T>*
                return val.get();
            }
            else
            {
                //const T*
                return &val;
            }
        }

        void onAdded(const T & val) override
        {
            auto& val_ref = *object_address(val);

            auto i = _set.find(std::invoke(foreign_key, val_ref));

            if (i != _set.end())
            {
                ValueSet & vs = *i;
                const bool is_new = vs.insert(valueToPointer(val)).second;
                assert(is_new);
                static_cast<void>(is_new);
            }
            else
            {
                ValueSet vs{ PrimaryCompare{} };
                vs.insert(valueToPointer(val));
                const bool is_new = _set.insert(std::move(vs)).second;
                assert(is_new);
                static_cast<void>(is_new);
            }
        }

        void onRemoving(const T & val) override
        {
            auto& val_ref = *object_address(val);
            
            auto i = _set.find(std::invoke(foreign_key, val_ref));

            assert(i != _set.end());

            ValueSet & vs = *i;

            assert(!vs.empty());

            if (vs.size() == 1)
            {
                assert(std::invoke(primary_key, *vs.front()) == std::invoke(primary_key, val_ref));
                
                //vs destructor will fire 'onClearing'.
                _set.erase(vs);
            }
            else
            {
                auto j = vs.find(std::invoke(primary_key, val_ref));

                assert(j != vs.end());
                
                vs.erase(j);
            }
        }

        void onClearing() override
        {
            _set.clear();
        }

        MultiSet _set;
    };
}
