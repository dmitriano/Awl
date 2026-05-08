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

        explicit OptionalStorage(std::shared_ptr<Logger> logger) :
            _logger(std::move(logger)),
            _storage(_logger)
        {}

        OptionalStorage(std::shared_ptr<Logger> logger, const awl::String& file_name, const awl::String& backup_name) :
            OptionalStorage(std::move(logger))
        {
            open(file_name, backup_name);
        }

        OptionalStorage(const OptionalStorage&) = delete;
        OptionalStorage(OptionalStorage&&) = default;

        OptionalStorage& operator = (const OptionalStorage&) = delete;

        OptionalStorage& operator = (OptionalStorage&& other)
        {
            _storage = std::move(other._storage);
            return *this;
        }

        bool isEmpty() const
        {
            return _storage.isEmpty();
        }

        bool isOpened() const
        {
            return _storage.isOpened();
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
            if (_storage.isOpened())
            {
                try
                {
                    return std::invoke(func, _storage);
                }
                catch (const IoException& e)
                {
                    _logger->warning(_T("Application settings were not saved correctly. Error message: {}"), e.message());
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

        std::shared_ptr<Logger> _logger;
        AtomicStorage _storage;
    };
}
