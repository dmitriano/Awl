#pragma once

#include "Awl/Io/HeaderedSerializable.h"
#include "Awl/Io/AtomicStorage.h"
#include "Awl/Mp/Mp.h"
#include "Awl/Logger.h"

namespace awl::io
{
    namespace helpers
    {
        bool LoadFromStorage(Serializable<>& val, AtomicStorage& storage, awl::Logger& logger,
            const awl::String& file_name, const awl::String& backup_name, bool allow_default);
    }

    template <class T, class Hash = awl::crypto::Crc64, class V = mp::variant_from_struct<T>>
    class PersistentObject
    {
    public:

        PersistentObject(awl::Logger& logger, Header header, size_t block_size = defaultBlockSize,
            Hash hash = {}, size_t format_name_limit = 64u)
        :
            m_logger(logger),
            m_serializable(std::move(header), m_val, block_size, std::move(hash), format_name_limit),
            m_storage(logger)
        {}

        bool load(const awl::String& file_name, bool allow_default = true)
        {
            const awl::String master_name = file_name + _T(".dat");
            const awl::String backup_name = file_name + _T(".bak");

            return helpers::LoadFromStorage(m_serializable, m_storage, m_logger, master_name, backup_name, allow_default);
        }

        bool load(const awl::String& file_name, const awl::String& backup_name, bool allow_default = true)
        {
            return helpers::LoadFromStorage(m_serializable, m_storage, m_logger, file_name, backup_name, allow_default);
        }

        void save()
        {
            if (m_storage.IsOpened())
            {
                m_storage.Save(m_serializable);
            }
        }

        void close()
        {
            m_storage.Close();
        }

        const T& value() const noexcept
        {
            return m_val;
        }

        T& value() noexcept
        {
            return m_val;
        }

        bool has_value() const noexcept
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

        awl::Logger& m_logger;

        T m_val;

        HeaderedSerializable<T, SequentialInputStream, SequentialOutputStream, Hash, V> m_serializable;

        AtomicStorage m_storage;
    };
}
