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

        bool Load(Value& val, const awl::String& file_name, const awl::String& backup_name);

        void Save(const Value& val);

    private:

        static void ReadFromStream(UniqueStream& s, Value& val)
        {
            val.Read(s);
        }

        static void WriteToStream(UniqueStream& s, const Value& val)
        {
            s.Seek(0);

            val.Write(s);

            s.Truncate();
            s.Flush();
        }

        std::tuple<awl::io::UniqueStream, bool> LoadFromFile(Value& val, const awl::String& file_name, LogLevel level);

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
