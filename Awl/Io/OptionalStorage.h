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
            return CallIfOpened(std::bind(&AtomicStorage::Load, std::placeholders::_1, std::ref(val)));
        }

        void Save(const Value& val)
        {
            CallIfOpened(std::bind(&AtomicStorage::Save, std::placeholders::_1, std::ref(val)));
        }

        void StartSave(const Value& val)
        {
            CallIfOpened(std::bind(&AtomicStorage::StartSave, std::placeholders::_1, std::ref(val)));
        }

        void StartSaveLocked(const Value& val, IMutex& mutex)
        {
            CallIfOpened(std::bind(&AtomicStorage::StartSaveLocked, std::placeholders::_1, std::ref(val), std::ref(mutex)));
        }

        void Wait()
        {
            CallIfOpened(std::bind(&AtomicStorage::Wait, std::placeholders::_1));
        }

        void Close()
        {
            CallIfOpened(std::bind(&AtomicStorage::Close, std::placeholders::_1));
        }

    private:

        template <class Func>
        auto CallIfOpened(Func func) -> std::invoke_result_t<Func, AtomicStorage*>
        {
            if (m_storage.IsOpened())
            {
                try
                {
                    return std::invoke(func, m_storage);
                }
                catch (const IoException& e)
                {
                    m_logger.warning(awl::format() << _T("Application settings were not saved correctly. ") <<
                        _T("Error message: ") << e.What());
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
