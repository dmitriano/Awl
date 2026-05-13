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

    inline std::move_only_function<void()> wrapInTransaction(
        std::unique_ptr<ITransaction> transaction,
        std::move_only_function<void()> func)
    {
        return [transaction = std::move(transaction), func = std::move(func)]() mutable
        {
            func();
            transaction->commit();
        };
    }
}
