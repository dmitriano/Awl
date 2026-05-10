#include "Awl/StringFormat.h"
#include "Awl/Testing/UnitTest.h"

#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/experimental/awaitable_operators.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <iostream>
#include <stdexcept>

namespace asio = boost::asio;
using asio::awaitable;
using asio::use_awaitable;
using asio::co_spawn;

namespace
{
    awaitable<void> task()
    {
        std::cout << "task(): before throw\n";
        throw std::runtime_error("boom from task");
        co_return;
    }

    awaitable<void> runner1(asio::io_context& io)
    {
        try
        {
            // ВАЖНО:
            // co_spawn(io, task(), use_awaitable) возвращает awaitable<void>.
            co_await co_spawn(io, task(), use_awaitable);

            std::cout << "task() finished without exception\n";
        }
        catch (const std::exception& e)
        {
            std::cout << "runner(): caught exception: " << e.what() << "\n";
        }
        catch (...)
        {
            std::cout << "runner(): caught unknown exception\n";
        }

        co_return;
    }

    awaitable<void> runner2(asio::io_context& io)
    {
        try
        {
            using namespace boost::asio::experimental::awaitable_operators;

            co_await (co_spawn(io, task(), use_awaitable) && co_spawn(io, task(), use_awaitable));

            std::cout << "task() finished without exception\n";
        }
        catch (const std::exception& e)
        {
            std::cout << "runner(): caught exception: " << e.what() << "\n";
        }
        catch (...)
        {
            std::cout << "runner(): caught unknown exception\n";
        }

        co_return;
    }
}

AWL_EXAMPLE(SpawnException1)
{
    AWL_UNUSED_CONTEXT;

    asio::io_context io;

    co_spawn(io, runner1(io), asio::detached);

    io.run();
}

AWL_EXAMPLE(SpawnException2)
{
    AWL_UNUSED_CONTEXT;

    asio::io_context io;

    co_spawn(io, runner2(io), asio::detached);

    io.run();
}
