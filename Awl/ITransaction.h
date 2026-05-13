#pragma once

namespace awl
{
    class ITransaction
    {
    public:

        virtual ~ITransaction() = default;

        virtual void commit() = 0;
    };
}
