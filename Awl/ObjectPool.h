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
            
            if (!m_free.empty())
            {
                p = m_free.pop_front();
            }
            else
            {
                p = new T();
            }

            return add(p);
        }

        std::shared_ptr<T> add(T* p)
        {
            m_used.push_back(p);

            return MakePointer(p);
        }

        ~object_pool()
        {
            assert(m_used.empty());

            clear();
        }

        void clear()
        {
            while (!m_free.empty())
            {
                delete m_free.pop_front();
            }
        }

    private:

        struct Deleter
        {
            object_pool* p_this;

            void operator () (T* p)
            {
                p_this->m_used.erase(p);
                //p->pooled_object::exclude();
                p_this->m_free.push_back(p);
                p->Finalize();
            }
        };

        friend Deleter;
        
        auto MakeDeleter()
        {
            return Deleter{ this };
        }

        std::shared_ptr<T> MakePointer(T* p)
        {
            return std::shared_ptr<T>(p, MakeDeleter());
        }

        using List = awl::quick_list<T, pooled_object_link>;
        
        List m_free;
        List m_used;
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
