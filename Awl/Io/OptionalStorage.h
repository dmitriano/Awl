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
            open(file_name, backup_name);
        }

        OptionalStorage(const OptionalStorage&) = delete;
        OptionalStorage(OptionalStorage&&) = default;

        OptionalStorage& operator = (const OptionalStorage&) = delete;

        OptionalStorage& operator = (OptionalStorage&& other)
        {
            m_storage = std::move(other.m_storage);
            return *this;
        }

        bool isEmpty() const
        {
            return m_storage.isEmpty();
        }

        bool isOpened() const
        {
            return m_storage.isOpened();
        }

        bool open(const awl::String& file_name, const awl::String& backup_name);

        bool load(Value& val)
        {
            return callIfOpened(std::bind(&AtomicStorage::load, std::placeholders::_1, std::ref(val)));
        }

        void save(const Value& val)
        {
            callIfOpened(std::bind(&AtomicStorage::save, std::placeholders::_1, std::ref(val)));
        }

        void startSave(const Value& val)
        {
            callIfOpened(std::bind(&AtomicStorage::startSave, std::placeholders::_1, std::ref(val)));
        }

        void startSaveLocked(const Value& val, IMutex& mutex)
        {
            callIfOpened(std::bind(&AtomicStorage::startSaveLocked, std::placeholders::_1, std::ref(val), std::ref(mutex)));
        }

        void wait()
        {
            callIfOpened(std::bind(&AtomicStorage::wait, std::placeholders::_1));
        }

        void close()
        {
            callIfOpened(std::bind(&AtomicStorage::close, std::placeholders::_1));
        }

    private:

        template <class Func>
        auto callIfOpened(Func func) -> std::invoke_result_t<Func, AtomicStorage*>
        {
            if (m_storage.isOpened())
            {
                try
                {
                    return std::invoke(func, m_storage);
                }
                catch (const IoException& e)
                {
                    m_logger.warning(_T("Application settings were not saved correctly. Error message: {}"), e.message());
                }
            }

            if constexpr (std::is_same_v<std::invoke_result_t<Func, AtomicStorage*>, void>)
            {
                return;
            }
            else
            {
                // This retrns false if the type is bool.
                return {};
            }
        }

        Logger& m_logger;
        AtomicStorage m_storage;
    };
}
