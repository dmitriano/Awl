#include "Awl/StringFormat.h"
#include "Awl/Testing/UnitTest.h"

#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>
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
    // Корутина, которая кидает exception
    awaitable<void> task()
    {
        std::cout << "task(): before throw\n";
        throw std::runtime_error("boom from task");
        co_return;
    }

    // "верхняя" корутина, которая делает co_await co_spawn(... use_awaitable)
    // и ловит исключение
    awaitable<void> runner(asio::io_context& io)
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
}

AWL_EXAMPLE(SpawnException)
{
    AWL_UNUSED_CONTEXT;

    asio::io_context io;

    // Запускаем верхнюю корутину runner(io)
    co_spawn(io, runner(io), asio::detached);

    io.run();
}
