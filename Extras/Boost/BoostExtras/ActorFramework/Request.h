#pragma once

#include <boost/asio/any_completion_handler.hpp>
#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/append.hpp>
#include <boost/asio/associated_allocator.hpp>
#include <boost/asio/async_result.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/system/system_error.hpp>

#include <atomic>
#include <exception>
#include <expected>
#include <memory>
#include <type_traits>
#include <utility>

namespace awl
{
    // A copyable reply address, not a queue. The first completion wins, including
    // admission errors. Results are posted to the requesting coroutine's executor.
    template <class T>
    class RequestCompletion
    {
    public:

        using Result = T;
        using Response = std::expected<T, std::exception_ptr>;

    private:

        class State
        {
        public:

            template <class Handler>
            State(Handler handler, const boost::asio::any_io_executor reply_executor,
                const boost::asio::any_io_executor target_executor) :
                _executor(reply_executor),
                _replyWork(boost::asio::make_work_guard(reply_executor)),
                _targetWork(boost::asio::make_work_guard(target_executor)),
                _handler(boost::asio::bind_executor(reply_executor, std::move(handler)))
            {}

            // Dropping an unanswered request must resume its caller, not silently
            // destroy the suspended coroutine. Executors must still be running.
            ~State()
            {
                if (!_completed.load())
                {
                    complete(std::unexpected(std::make_exception_ptr(
                        boost::system::system_error(boost::asio::error::operation_aborted))));
                }
            }

            bool complete(Response response)
            {
                if (_completed.exchange(true))
                {
                    return false;
                }

                boost::asio::post(_executor,
                    boost::asio::append(std::move(_handler), std::move(response)));
                _replyWork.reset();
                _targetWork.reset();
                return true;
            }

        private:

            const boost::asio::any_io_executor _executor;
            boost::asio::executor_work_guard<boost::asio::any_io_executor> _replyWork;
            boost::asio::executor_work_guard<boost::asio::any_io_executor> _targetWork;
            boost::asio::any_completion_handler<void(Response)> _handler;
            std::atomic<bool> _completed = false;
        };

    public:

        RequestCompletion() = default;

        template <class Handler>
        static RequestCompletion create(Handler handler,
            const boost::asio::any_io_executor reply_executor,
            const boost::asio::any_io_executor target_executor)
        {
            const auto allocator = boost::asio::get_associated_allocator(handler);
            RequestCompletion completion;
            completion._state = std::allocate_shared<State>(allocator, std::move(handler),
                reply_executor, target_executor);
            return completion;
        }

        // Copies may be held by the message and its admission callback. Completing
        // twice is harmless; a false result means another path already answered.
        bool complete(Response response) const
        {
            return _state->complete(std::move(response));
        }

        bool fail(const std::exception_ptr error) const
        {
            return complete(std::unexpected(error));
        }

    private:

        std::shared_ptr<State> _state;
    };

    // Request is an owned data message with `using Result = ...` and a default
    // RequestCompletion<Result> completion member. Inbox is an awl::Channel of
    // messages (typically a variant) accepting this request. No workers are created.
    // Once initiated, wait for admission and a reply without per-operation cancellation.
    // The component must finish/fail accepted requests before stopping its consumer.
    template <class Inbox, class Request>
    boost::asio::awaitable<typename Request::Result> asyncRequest(
        std::shared_ptr<Inbox> inbox, Request request)
    {
        namespace asio = boost::asio;
        using Result = typename Request::Result;
        using Completion = RequestCompletion<Result>;
        using Response = typename Completion::Response;

        const asio::any_io_executor executor = co_await asio::this_coro::executor;
        auto token = asio::use_awaitable;

        Response response = co_await asio::async_initiate<decltype(token), void(Response)>(
            [](auto handler, std::shared_ptr<Inbox> target, Request message,
                const asio::any_io_executor reply_executor)
            {
                message.completion = Completion::create(std::move(handler),
                    reply_executor, target->get_executor());
                const Completion completion = message.completion;

                target->async_send(boost::system::error_code{}, std::move(message),
                    [target, completion](const boost::system::error_code error)
                    {
                        if (error)
                        {
                            completion.fail(std::make_exception_ptr(boost::system::system_error(error)));
                        }
                    });
            }, token, std::move(inbox), std::move(request), executor);

        if (!response)
        {
            std::rethrow_exception(response.error());
        }

        if constexpr (!std::is_void_v<Result>)
        {
            co_return std::move(response).value();
        }
    }
}
