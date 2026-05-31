#pragma once

#include "Awl/Coro/Task.h"
#include "Awl/Exception.h"

#include <algorithm>
#include <coroutine>
#include <cstddef>
#include <deque>
#include <exception>
#include <memory>
#include <mutex>
#include <optional>
#include <utility>
#include <vector>

namespace awl::coro
{
    AWL_DEFINE_EXCEPTION(ChannelClosedException)

    template <class T>
    class Channel
    {
    private:

        class ReceiveOperation;
        class SendOperation;

        struct Continuation
        {
            std::shared_ptr<IDispatcher> dispatcher;
            std::coroutine_handle<> coroutine;
        };

        struct State
        {
            explicit State(const size_t init_capacity) :
                capacity(init_capacity)
            {}

            std::mutex mutex;
            bool open = true;
            size_t capacity;
            std::deque<T> values;
            std::deque<ReceiveOperation*> receivers;
            std::deque<SendOperation*> senders;
        };

    public:

        explicit Channel(size_t capacity = 0) :
            _state(std::make_shared<State>(capacity))
        {}

        Channel(const Channel&) = delete;

        Channel(Channel&&) = delete;

        Channel& operator = (const Channel&) = delete;

        Channel& operator = (Channel&&) = delete;

        ~Channel()
        {
            close();
        }

        bool isOpen() const
        {
            std::lock_guard lock(_state->mutex);

            return _state->open;
        }

        void close()
        {
            std::vector<Continuation> continuations;

            {
                std::lock_guard lock(_state->mutex);

                if (_state->open)
                {
                    _state->open = false;

                    while (!_state->receivers.empty())
                    {
                        ReceiveOperation* receiver = _state->receivers.front();
                        _state->receivers.pop_front();

                        receiver->setException(makeClosedException(), continuations);
                    }

                    while (!_state->senders.empty())
                    {
                        SendOperation* sender = _state->senders.front();
                        _state->senders.pop_front();

                        sender->setException(makeClosedException(), continuations);
                    }
                }
            }

            resume(continuations);
        }

        Task<void> asyncSend(T val)
        {
            co_await SendOperation(_state, std::move(val));
        }

        Task<T> asyncReceive()
        {
            co_return co_await ReceiveOperation(_state);
        }

    private:

        static std::exception_ptr makeClosedException()
        {
            return std::make_exception_ptr(ChannelClosedException("Channel is closed."));
        }

        static void resume(const std::vector<Continuation>& continuations)
        {
            for (const Continuation& continuation : continuations)
            {
                detail::resume(continuation.dispatcher, continuation.coroutine);
            }
        }

        template <class Operation>
        static void erase(std::deque<Operation*>& operations, Operation* operation);

        static void complete(SendOperation& sender, std::vector<Continuation>& continuations);

        static void complete(
            ReceiveOperation& receiver,
            T val,
            std::vector<Continuation>& continuations);

        static void moveWaitingSenderToBuffer(
            State& state,
            std::vector<Continuation>& continuations);

        class ReceiveOperation
        {
        public:

            explicit ReceiveOperation(std::shared_ptr<State> init_state) :
                state(std::move(init_state))
            {}

            ReceiveOperation(const ReceiveOperation&) = delete;

            ReceiveOperation(ReceiveOperation&&) = delete;

            ReceiveOperation& operator = (const ReceiveOperation&) = delete;

            ReceiveOperation& operator = (ReceiveOperation&&) = delete;

            ~ReceiveOperation()
            {
                unsubscribe();
            }

            bool await_ready() const noexcept
            {
                return false;
            }

            void setDispatcher(const std::shared_ptr<IDispatcher>& init_dispatcher)
            {
                dispatcher = init_dispatcher;
            }

            bool await_suspend(std::coroutine_handle<> h)
            {
                coroutine = h;

                std::vector<Continuation> continuations;
                bool suspended = false;

                {
                    std::lock_guard lock(state->mutex);

                    if (!state->values.empty())
                    {
                        value = std::move(state->values.front());
                        state->values.pop_front();
                        coroutine = nullptr;

                        moveWaitingSenderToBuffer(*state, continuations);
                    }
                    else if (!state->senders.empty())
                    {
                        SendOperation* sender = state->senders.front();
                        state->senders.pop_front();

                        value = std::move(*sender->value);
                        sender->value.reset();
                        coroutine = nullptr;

                        complete(*sender, continuations);
                    }
                    else if (!state->open)
                    {
                        exception = makeClosedException();
                        coroutine = nullptr;
                    }
                    else
                    {
                        state->receivers.push_back(this);
                        registered = true;
                        suspended = true;
                    }
                }

                resume(continuations);

                return suspended;
            }

            T await_resume()
            {
                unsubscribe();

                if (exception)
                {
                    std::rethrow_exception(exception);
                }

                return std::move(*value);
            }

            void setException(
                std::exception_ptr init_exception,
                std::vector<Continuation>& continuations)
            {
                registered = false;
                exception = std::move(init_exception);

                if (std::coroutine_handle<> continuation = std::exchange(coroutine, nullptr))
                {
                    continuations.push_back({ dispatcher, continuation });
                }
            }

            std::shared_ptr<State> state;
            std::shared_ptr<IDispatcher> dispatcher;
            std::optional<T> value;
            std::exception_ptr exception;
            std::coroutine_handle<> coroutine = nullptr;
            bool registered = false;

        private:

            void unsubscribe()
            {
                if (registered)
                {
                    std::lock_guard lock(state->mutex);

                    erase(state->receivers, this);
                    registered = false;
                    coroutine = nullptr;
                }
            }
        };

        class SendOperation
        {
        public:

            SendOperation(std::shared_ptr<State> init_state, T init_value) :
                state(std::move(init_state)),
                value(std::move(init_value))
            {}

            SendOperation(const SendOperation&) = delete;

            SendOperation(SendOperation&&) = delete;

            SendOperation& operator = (const SendOperation&) = delete;

            SendOperation& operator = (SendOperation&&) = delete;

            ~SendOperation()
            {
                unsubscribe();
            }

            bool await_ready() const noexcept
            {
                return false;
            }

            void setDispatcher(const std::shared_ptr<IDispatcher>& init_dispatcher)
            {
                dispatcher = init_dispatcher;
            }

            bool await_suspend(std::coroutine_handle<> h)
            {
                coroutine = h;

                std::vector<Continuation> continuations;
                bool suspended = false;

                {
                    std::lock_guard lock(state->mutex);

                    if (!state->open)
                    {
                        exception = makeClosedException();
                        coroutine = nullptr;
                    }
                    else if (!state->receivers.empty())
                    {
                        ReceiveOperation* receiver = state->receivers.front();
                        state->receivers.pop_front();

                        complete(*receiver, std::move(*value), continuations);
                        value.reset();
                        coroutine = nullptr;
                    }
                    else if (state->capacity != 0 && state->values.size() < state->capacity)
                    {
                        state->values.push_back(std::move(*value));
                        value.reset();
                        coroutine = nullptr;
                    }
                    else
                    {
                        state->senders.push_back(this);
                        registered = true;
                        suspended = true;
                    }
                }

                resume(continuations);

                return suspended;
            }

            void await_resume()
            {
                unsubscribe();

                if (exception)
                {
                    std::rethrow_exception(exception);
                }
            }

            void setException(
                std::exception_ptr init_exception,
                std::vector<Continuation>& continuations)
            {
                registered = false;
                exception = std::move(init_exception);

                if (std::coroutine_handle<> continuation = std::exchange(coroutine, nullptr))
                {
                    continuations.push_back({ dispatcher, continuation });
                }
            }

            std::shared_ptr<State> state;
            std::shared_ptr<IDispatcher> dispatcher;
            std::optional<T> value;
            std::exception_ptr exception;
            std::coroutine_handle<> coroutine = nullptr;
            bool registered = false;

        private:

            void unsubscribe()
            {
                if (registered)
                {
                    std::lock_guard lock(state->mutex);

                    erase(state->senders, this);
                    registered = false;
                    coroutine = nullptr;
                }
            }
        };

        std::shared_ptr<State> _state;
    };

    template <class T>
    template <class Operation>
    void Channel<T>::erase(std::deque<Operation*>& operations, Operation* operation)
    {
        const auto i = std::ranges::find(operations, operation);

        if (i != operations.end())
        {
            operations.erase(i);
        }
    }

    template <class T>
    void Channel<T>::complete(
        typename Channel<T>::SendOperation& sender,
        std::vector<Continuation>& continuations)
    {
        sender.registered = false;

        if (std::coroutine_handle<> continuation = std::exchange(sender.coroutine, nullptr))
        {
            continuations.push_back({ sender.dispatcher, continuation });
        }
    }

    template <class T>
    void Channel<T>::complete(
        typename Channel<T>::ReceiveOperation& receiver,
        T val,
        std::vector<Continuation>& continuations)
    {
        receiver.registered = false;
        receiver.value = std::move(val);

        if (std::coroutine_handle<> continuation = std::exchange(receiver.coroutine, nullptr))
        {
            continuations.push_back({ receiver.dispatcher, continuation });
        }
    }

    template <class T>
    void Channel<T>::moveWaitingSenderToBuffer(
        typename Channel<T>::State& state,
        std::vector<Continuation>& continuations)
    {
        if (state.open && state.capacity != 0 && !state.senders.empty() && state.values.size() < state.capacity)
        {
            typename Channel<T>::SendOperation* sender = state.senders.front();
            state.senders.pop_front();

            state.values.push_back(std::move(*sender->value));
            sender->value.reset();

            complete(*sender, continuations);
        }
    }
}
