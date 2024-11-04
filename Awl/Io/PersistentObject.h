#pragma once

#include "Awl/Io/HeaderedSerializable.h"
#include "Awl/Io/AtomicStorage.h"
#include "Awl/Logger.h"

namespace awl::io
{
    namespace helpers
    {
        bool LoadFromStorage(Serializable<>& val, AtomicStorage& storage, awl::Logger& logger,
            const awl::String& file_name, const awl::String& backup_name, bool allow_default);
    }

    template <class T, class V, class Hash = awl::crypto::Crc64>
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

        bool load(const awl::String& file_name, const awl::String& backup_name, bool allow_default = true)
        {
            return helpers::LoadFromStorage(m_serializable, m_storage, m_logger, file_name, backup_name, allow_default);
        }

        void save()
        {
            m_storage.Save(m_serializable);
        }

        const T& value() const
        {
            return m_val;
        }

        T& value()
        {
            return m_val;
        }

        bool has_value() const
        {
            return m_storage.IsOpened();
        }

        const T& operator * () const
        {
            return m_val;
        }

        const T* operator -> () const
        {
            return &m_val;
        }

        T& operator * ()
        {
            return m_val;
        }

        T* operator -> ()
        {
            return &m_val;
        }

    private:

        awl::Logger& m_logger;

        T m_val;

        HeaderedSerializable<T, V, Hash> m_serializable;

        AtomicStorage m_storage;
    };
}
