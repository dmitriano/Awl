#include "Awl/Testing/UnitTest.h"

#include <boost/asio.hpp>

#include <iostream>

namespace asio = boost::asio;
using asio::awaitable;
using asio::use_awaitable;

namespace
{
    class Param
    {
    public:

        Param(int val) : m_val(val)
        {
            std::cout << "Param constructor " << m_val << std::endl;
        }

        Param(const Param& other) : m_val(other.m_val)
        {
            std::cout << "Param copy constructor " << m_val << std::endl;
        }

        Param(Param&& other) noexcept : m_val(other.m_val)
        {
            std::cout << "Param move constructor " << m_val << std::endl;
        }

        Param& operator= (const Param& other)
        {
            m_val = other.m_val;

            std::cout << "Param copy assignment operator " << m_val << std::endl;

            return *this;
        }

        Param& operator= (Param&& other) noexcept
        {
            m_val = other.m_val;

            std::cout << "Param move assignment operator " << m_val << std::endl;

            return *this;
        }

        void func() const
        {
            std::cout << "Param func " << m_val << std::endl;
        }

        ~Param()
        {
            std::cout << "Param destructor " << m_val << std::endl;
        }

        int value() const
        {
            return m_val;
        }

        void setValue(int val)
        {
            m_val = val;
        }

    private:

        int m_val;
    };

    void simpleFunc(Param param)
    {
        std::cout << "Simple func ";

        param.func();
    }

    awaitable<void> coroFunc(Param param)
    {
        auto exec = co_await asio::this_coro::executor;

        asio::steady_timer timer{ exec };

        timer.expires_after(std::chrono::milliseconds(100));

        co_await timer.async_wait(use_awaitable);

        param.func();

        co_return;
    }

    awaitable<void> callerFunc()
    {
        // Copy elision, No move no assignment.
        simpleFunc(10);
        
        // Move constructor is called.
        co_await coroFunc(1);

        std::cout << std::endl;

        {
            Param param(2);

            // Copy constructor is called.
            simpleFunc(param);

            // Copy and Move constructors are called.
            co_await coroFunc(param);
        }

        std::cout << std::endl;

        {
            Param param(3);

            std::cout << "Initialized param " << param.value() << std::endl;

            co_await coroFunc(param);

            param.setValue(4);

            std::cout << "Updated param " << param.value() << std::endl;
        }
    }

    // awaitable<void> coroFuncByRef(const Param& param)
    // {
    //     param.func();
    //
    //     co_return;
    // }

    // awaitable<void> callerFuncUb()
    // {
    //     auto task = coroFuncByRef(1);
    //
    //     co_await std::move(task);
    // }
}

AWL_EXAMPLE(CoroParam)
{
    AWL_UNUSED_CONTEXT;

    asio::thread_pool pool;

    asio::co_spawn(pool, callerFunc(), asio::detached);

    pool.join();
}
