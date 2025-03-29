#pragma once

namespace awl
{
    struct IMutex
    {
        virtual void lock() = 0;
        virtual void unlock() = 0;
    };

    template <class mutex>
    class MutexWrapper : public IMutex
    {
    public:

        MutexWrapper(mutex& wrapped) : m(wrapped) {}

        void lock() override
        {
            m.lock();
        }

        void unlock() override
        {
            m.unlock();
        }

    private:

        mutex& m;
    };

    template <class mutex>
    class OptionalMutex
    {
    public:

        OptionalMutex(mutex* p_m) : pMutex(p_m) {}

        void lock()
        {
            if (pMutex != nullptr)
            {
                pMutex->lock();
            }
        }

        void unlock()
        {
            if (pMutex != nullptr)
            {
                pMutex->unlock();
            }
        }

    private:

        mutex* pMutex;
    };

    // We also need a fake mutex in addition to std::mutex and std::recursive_mutex, etc...
    // It can be used as a default value of a template class parameter, for example.
    struct fake_mutex
    {
        void lock() {}
        void unlock() {}
    };

    // Fake mutex wrapper that implements IMutex interface.
    class FakeMutex :
        private fake_mutex,
        public MutexWrapper<fake_mutex>
    {
    public:

        FakeMutex() : MutexWrapper<fake_mutex>(*this) {}
    };
}
