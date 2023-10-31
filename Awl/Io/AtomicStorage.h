/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Logger.h"
#include "Awl/Io/Serializable.h"
#include "Awl/Io/NativeStream.h"
#include <cassert>

namespace awl::io
{
    class AtomicStorage
    {
    private:

        using Value = Serializable<UniqueStream, UniqueStream>;

    public:

        AtomicStorage(Logger& logger) : m_logger(logger) {}

        AtomicStorage(Logger& logger, const awl::String& file_name, const awl::String& backup_name) : 
            AtomicStorage(logger)
        {
            Open(file_name, backup_name);
        }

        AtomicStorage(const AtomicStorage&) = delete;
        AtomicStorage(AtomicStorage&&) = default;

        AtomicStorage& operator = (const AtomicStorage&) = delete;

        AtomicStorage& operator = (AtomicStorage&& other) noexcept
        {
            m_s = std::move(other.m_s);
            m_backup = std::move(other.m_backup);
            return *this;
        }

        bool IsEmpty() const
        {
            assert(IsOpened());
            return m_s.GetLength() == 0 && m_backup.GetLength() == 0;
        }

        bool IsOpened() const
        {
            return m_s != UniqueStream{};
        }

        bool Open(const awl::String& file_name, const awl::String& backup_name)
        {
            m_s = awl::io::CreateUniqueFile(file_name);
            const bool master_existed = OpenedExisting();

            m_backup = awl::io::CreateUniqueFile(backup_name);
            const bool backup_existed = OpenedExisting();

            return master_existed || backup_existed;
        }

        bool Load(Value& val);

        void Save(const Value& val);

        void Close()
        {
            m_s = {};
            m_backup = {};
        }

    private:

        static void ReadFromStream(UniqueStream& s, Value& val)
        {
            s.Seek(0);

            val.Read(s);
        }

        static void WriteToStream(UniqueStream& s, const Value& val)
        {
            s.Seek(0);

            val.Write(s);

            s.Truncate();
            s.Flush();
        }

        bool LoadFromFile(Value& val, awl::io::UniqueStream& s, LogLevel level);

        void ClearBackup()
        {
            m_backup.Seek(0);
            m_backup.Truncate();
            m_backup.Flush();
        }

        Logger& m_logger;
        UniqueStream m_s;
        UniqueStream m_backup;
    };
}
