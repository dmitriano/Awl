#pragma once

#include "Awl/Io/HeaderedSerializable.h"
#include "Awl/Io/AtomicStorage.h"
#include "Awl/Io/OptionalStorage.h"
#include "Awl/Mp/Mp.h"
#include "Awl/ILogger.h"

#include <filesystem>
#include <mutex>
#include <memory>

namespace awl::io
{
    template <class T, class Storage = AtomicStorage, class Hash = awl::crypto::Crc64, class V = mp::variant_from_struct<T>>
    class PersistentObject
    {
    public:

        PersistentObject(std::shared_ptr<awl::ILogger> logger, Header header, size_t block_size = defaultBlockSize,
            Hash hash = {}, size_t format_name_limit = 64u)
        :
            _serializable(std::move(header), _val, block_size, std::move(hash), format_name_limit),
            _storage(std::move(logger))
        {}

        bool open(const awl::String& file_name)
        {
            auto [master_name, backup_name] = append_extensions(file_name);

            return _storage.open(master_name, backup_name);
        }

        bool load()
        {
            return _storage.load(_serializable);
        }

        void save()
        {
            _storage.startSave(_serializable);
        }

        void wait()
        {
            _storage.wait();
        }

        void close()
        {
            _storage.close();
        }

        void remove(const awl::String& file_name)
        {
            close();

            auto [master_name, backup_name] = append_extensions(file_name);

            namespace fs = std::filesystem;

            fs::remove(master_name);
            fs::remove(backup_name);
        }

        const T& value() const noexcept
        {
            return _val;
        }

        T& value() noexcept
        {
            return _val;
        }

        bool is_open() const noexcept
        {
            return _storage.isOpened();
        }

        const T& operator * () const noexcept
        {
            return _val;
        }

        T& operator * () noexcept
        {
            return _val;
        }

        const T* operator -> () const noexcept
        {
            return &_val;
        }

        T* operator -> () noexcept
        {
            return &_val;
        }

    private:

        auto append_extensions(const awl::String& file_name)
        {
            const awl::String master_name = file_name + _T(".dat");
            const awl::String backup_name = file_name + _T(".bak");

            return std::make_tuple(master_name, backup_name);
        }

        std::mutex _mutex;

        T _val;

        HeaderedSerializable<T, SequentialInputStream, SequentialOutputStream, Hash, V> _serializable;

        Storage _storage;
    };

    template <class T, class Hash = awl::crypto::Crc64, class V = mp::variant_from_struct<T>>
    using OptionalObject = PersistentObject<T, OptionalStorage, Hash, V>;
}
