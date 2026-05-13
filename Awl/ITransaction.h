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

    class ITransactionFactory
    {
    public:

        virtual ~ITransactionFactory() = default;

        virtual std::unique_ptr<awl::ITransaction> startTransaction() = 0;
    };

    inline std::move_only_function<void()> wrapInTransaction(
        ITransactionFactory& transaction_factory,
        std::move_only_function<void()> func)
    {
        return [&transaction_factory, func = std::move(func)]() mutable
        {
            std::unique_ptr<ITransaction> transaction = transaction_factory.startTransaction();
            func();
            transaction->commit();
        };
    }
}
