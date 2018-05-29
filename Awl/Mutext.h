#pragma once

#include <mutex>

namespace awl
{
    struct IMutex
    {
        virtual void Lock() = 0;
        virtual void Unlock() = 0;
    };

    template <class mutex>
    class MutexWrapper : public IMutex
    {
    public:

        MutexWrapper(mutex & wrapped) : m(wrapped)
        {
        }

        void Lock() override
        {
            m.lock();
        }

        void Unlock() override
        {
            m.unlock();
        }

    private:

        mutex & m;
    };

    //typedef Mutex<std::mutex> Mutex;
    //typedef Mutex<std::recursive_mutex> RecursiveMutex;

    class OptionalLock
    {
    public:

        OptionalLock(IMutex * p_mutex) : pMutex(p_mutex)
        {
            if (pMutex != nullptr)
            {
                pMutex->Lock();
            }
        }

        ~OptionalLock()
        {
            if (pMutex != nullptr)
            {
                pMutex->Unlock();
            }
        }

        void Unlock()
        {
            if (pMutex != nullptr)
            {
                pMutex->Unlock();

                pMutex = nullptr;
            }
        }

    private:

        IMutex * pMutex;
    };
}
