/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/QuickList.h"
#include "Awl/PooledObject.h"

#include <memory>
#include <cassert>

namespace awl
{
    //T is default constructible and derived from awl::quick_link.
    template <class T>
    class object_pool
    {
    public:

        using value_type = T;
        
        std::shared_ptr<T> make()
        {
            T* p;
            
            if (!_free.empty())
            {
                p = _free.pop_front();
            }
            else
            {
                p = new T();
            }

            return add(p);
        }

        std::shared_ptr<T> add(T* p)
        {
            _used.push_back(p);

            return makePointer(p);
        }

        ~object_pool()
        {
            assert(_used.empty());

            clear();
        }

        void clear()
        {
            while (!_free.empty())
            {
                delete _free.pop_front();
            }
        }

    private:

        struct Deleter
        {
            object_pool* p_this;

            void operator () (T* p)
            {
                p_this->_used.erase(p);
                //p->pooled_object::exclude();
                p_this->_free.push_back(p);
                p->finalize();
            }
        };

        friend Deleter;
        
        auto makeDeleter()
        {
            return Deleter{ this };
        }

        std::shared_ptr<T> makePointer(T* p)
        {
            return std::shared_ptr<T>(p, makeDeleter());
        }

        using List = awl::quick_list<T, pooled_object_link>;
        
        List _free;
        List _used;
    };

    template <class T>
    inline object_pool<T> objectPoolSingleton;

    template <class T>
    std::shared_ptr<T> make_pooled()
    {
        return objectPoolSingleton<T>.make();
    }

    template <class T>
    void clear_pool()
    {
        objectPoolSingleton<T>.clear();
    }
}
