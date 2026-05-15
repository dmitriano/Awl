#pragma once

#include "Awl/Coro/ISharedExecutor.h"
#include "Awl/ITransaction.h"

#include <functional>
#include <memory>
#include <utility>

namespace awl::coro
{
    class TransactDispatcher : public ISharedDispatcher
    {
    public:

        TransactDispatcher(
            std::shared_ptr<ISharedDispatcher> dispatcher,
            std::shared_ptr<awl::ITransactionProvider> transaction_provider) :
            _dispatcher(std::move(dispatcher)),
            _transactionProvider(std::move(transaction_provider))
        {}

        Awaitable<void> execute(std::move_only_function<void()> func, Options options) override
        {
            return _dispatcher->execute(wrap(std::move(func), options), options);
        }

        void post(std::move_only_function<void()> func, Options options) override
        {
            _dispatcher->post(wrap(std::move(func), options), options);
        }

        void join() override
        {
            _dispatcher->join();
        }

    private:

        std::move_only_function<void()> wrap(std::move_only_function<void()> func, Options& options) const
        {
            if (options[Option::Transaction])
            {
                options[Option::Transaction] = false;

                return awl::wrapInTransaction(_transactionProvider, std::move(func));
            }

            return std::move(func);
        }

        std::shared_ptr<ISharedDispatcher> _dispatcher;
        std::shared_ptr<awl::ITransactionProvider> _transactionProvider;
    };
}
