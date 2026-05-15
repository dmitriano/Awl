#pragma once

#include <functional>
#include <memory>
#include <utility>

namespace awl
{
    class ITransaction
    {
    public:

        virtual ~ITransaction() = default;

        virtual void commit() = 0;
    };

    class ITransactionProvider
    {
    public:

        virtual ~ITransactionProvider() = default;

        virtual std::unique_ptr<awl::ITransaction> startTransaction() = 0;
    };

    inline std::move_only_function<void()> wrapInTransaction(
        std::shared_ptr<ITransactionProvider> transaction_provider,
        std::move_only_function<void()> func)
    {
        return [transaction_provider = std::move(transaction_provider), func = std::move(func)]() mutable
        {
            std::unique_ptr<ITransaction> transaction = transaction_provider->startTransaction();
            func();
            transaction->commit();
        };
    }
}
