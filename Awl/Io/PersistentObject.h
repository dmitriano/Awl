#pragma once

#include "Awl/Io/HeaderedSerializable.h"
#include "Awl/Io/AtomicStorage.h"
#include "Awl/Io/OptionalStorage.h"
#include "Awl/Mp/Mp.h"
#include "Awl/Logger.h"

#include <filesystem>
#include <mutex>

namespace awl::io
{
    template <class T, class Storage = AtomicStorage, class Hash = awl::crypto::Crc64, class V = mp::variant_from_struct<T>>
    class PersistentObject
    {
    public:

        PersistentObject(awl::Logger& logger, Header header, size_t block_size = defaultBlockSize,
            Hash hash = {}, size_t format_name_limit = 64u)
        :
            m_serializable(std::move(header), m_val, block_size, std::move(hash), format_name_limit),
            m_storage(logger)
        {}

        bool open(const awl::String& file_name)
        {
            auto [master_name, backup_name] = append_extensions(file_name);

            return m_storage.Open(master_name, backup_name);
        }

        bool load()
        {
            return m_storage.Load(m_serializable);
        }

        void save()
        {
            m_storage.Save(m_serializable);
        }

        void start_save()
        {
            m_storage.StartSave(m_serializable);
        }

        void wait()
        {
            m_storage.Wait();
        }

        void close()
        {
            m_storage.Close();
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
            return m_val;
        }

        T& value() noexcept
        {
            return m_val;
        }

        bool is_open() const noexcept
        {
            return m_storage.IsOpened();
        }

        const T& operator * () const noexcept
        {
            return m_val;
        }

        T& operator * () noexcept
        {
            return m_val;
        }

        const T* operator -> () const noexcept
        {
            return &m_val;
        }

        T* operator -> () noexcept
        {
            return &m_val;
        }

    private:

        auto append_extensions(const awl::String& file_name)
        {
            const awl::String master_name = file_name + _T(".dat");
            const awl::String backup_name = file_name + _T(".bak");

            return std::make_tuple(master_name, backup_name);
        }

        std::mutex m_mutex;

        T m_val;

        HeaderedSerializable<T, SequentialInputStream, SequentialOutputStream, Hash, V> m_serializable;

        Storage m_storage;
    };

    template <class T, class Hash = awl::crypto::Crc64, class V = mp::variant_from_struct<T>>
    using OptionalObject = PersistentObject<T, OptionalStorage, Hash, V>;
}
