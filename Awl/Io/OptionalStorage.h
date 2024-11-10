/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/AtomicStorage.h"

namespace awl::io
{
    class OptionalStorage
    {
    private:

        using Value = Serializable<>;

    public:

        OptionalStorage(Logger& logger) : 
            m_logger(logger),
            m_storage(logger)
        {}

        OptionalStorage(Logger& logger, const awl::String& file_name, const awl::String& backup_name) : 
            OptionalStorage(logger)
        {
            Open(file_name, backup_name);
        }

        OptionalStorage(const OptionalStorage&) = delete;
        OptionalStorage(OptionalStorage&&) = default;

        OptionalStorage& operator = (const OptionalStorage&) = delete;

        OptionalStorage& operator = (OptionalStorage&& other)
        {
            m_storage = std::move(other.m_storage);
            return *this;
        }

        bool IsEmpty() const
        {
            return m_storage.IsEmpty();
        }

        bool IsOpened() const
        {
            return m_storage.IsOpened();
        }

        bool Open(const awl::String& file_name, const awl::String& backup_name);

        bool Load(Value& val)
        {
            return m_storage.Load(val);
        }

        void Save(const Value& val, IMutex* p_mutex = nullptr);

        void Close()
        {
            m_storage.Close();
        }

    private:

        Logger& m_logger;
        AtomicStorage m_storage;
    };
}
