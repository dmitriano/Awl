#pragma once

#include "BoostExtras/Channel.h"

#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/use_awaitable.hpp>

#include <exception>
#include <memory>
#include <utility>

namespace awl
{
    // Non-recursive coroutine mutex. Waiting suspends the coroutine, not its thread.
    // The concurrent channel allows callers on different executors; it creates no workers.
    class AsyncMutex
    {
    private:

        using State = Channel<void(boost::system::error_code)>;

    public:

        class Lock
        {
        public:

            Lock() = default;

            Lock(const Lock&) = delete;

            Lock& operator = (const Lock&) = delete;

            Lock(Lock&& other) noexcept : _state(std::move(other._state))
            {}

            Lock& operator = (Lock&& other) noexcept
            {
                if (this != &other)
                {
                    unlock();
                    _state = std::move(other._state);
                }

                return *this;
            }

            ~Lock()
            {
                unlock();
            }

            bool ownsLock() const noexcept { return static_cast<bool>(_state); }

            // Also safe on an empty or moved-from lock.
            void unlock() noexcept
            {
                if (_state)
                {
                    std::shared_ptr<State> state = std::move(_state);

                    if (!state->try_send(boost::system::error_code{}))
                    {
                        std::terminate();
                    }
                }
            }

        private:

            friend class AsyncMutex;

            explicit Lock(std::shared_ptr<State> state) : _state(std::move(state))
            {}

            std::shared_ptr<State> _state;
        };

        explicit AsyncMutex(const boost::asio::any_io_executor executor) :
            _state(std::make_shared<State>(executor, 1))
        {
            _state->try_send(boost::system::error_code{});
        }

        AsyncMutex(const AsyncMutex&) = delete;

        AsyncMutex& operator = (const AsyncMutex&) = delete;

        // Capture ownership before returning a lazy awaitable. Locks and already-created
        // waits keep the state alive independently of the AsyncMutex wrapper.
        [[nodiscard]] boost::asio::awaitable<Lock> asyncLock()
        {
            return asyncLockImpl(_state);
        }

    private:

        static boost::asio::awaitable<Lock> asyncLockImpl(std::shared_ptr<State> state)
        {
            co_await state->async_receive(boost::asio::use_awaitable);
            co_return Lock(std::move(state));
        }

        const std::shared_ptr<State> _state;
    };
}
