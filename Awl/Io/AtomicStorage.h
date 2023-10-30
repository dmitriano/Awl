/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Logger.h"
#include "Awl/Io/Serializable.h"
#include "Awl/Io/NativeStream.h"

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
        AtomicStorage& operator = (AtomicStorage&&) = delete;

        // TODO: Implement IsOpened().

        void Open(const awl::String& file_name, const awl::String& backup_name)
        {
            m_s = awl::io::CreateUniqueFile(file_name);
            m_backup = awl::io::CreateUniqueFile(backup_name);
        }

        bool Load(Value& val);

        void Save(const Value& val);

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
