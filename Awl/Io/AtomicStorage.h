/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Logger.h"
#include "Awl/OptionalMutex.h"
#include "Awl/Io/Serializable.h"
#include "Awl/Io/NativeStream.h"
#include "Awl/Io/Snapshotable.h"
#include <cassert>

namespace awl::io
{
    class AtomicStorage
    {
    private:

        // We open files as UniqueStream, but use them as basic SequentialInputStream, SequentialOutputStream.
        // Virtual functions will not add a significant overhead, because higher-level streams (like hashing stream)
        // read/write into their underlying streams by blocks.
        using Value = Serializable<>;

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
            try
            {
                m_s = awl::io::CreateUniqueFile(file_name);
                const bool master_existed = OpenedExisting();

                m_backup = awl::io::CreateUniqueFile(backup_name);
                const bool backup_existed = OpenedExisting();

                return master_existed || backup_existed;
            }
            catch (const IoException&)
            {
                Close();

                throw;
            }
        }

        bool Load(Value& val);

        void Save(const Value& val, IMutex* p_mutex = nullptr);

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

        template <class Func>
        static void WriteToStreamFunc(UniqueStream& s, Func&& func)
        {
            s.Seek(0);

            func(s);

            s.Truncate();
            s.Flush();
        }

        static void WriteToStream(UniqueStream& s, const Value& val)
        {
            WriteToStreamFunc(s, [&val](UniqueStream& s) { val.Write(s); });
        }

        static void WriteSnapshot(UniqueStream& s, const awl::io::Snapshotable<SequentialOutputStream>& snapshotable, const std::vector<uint8_t>& v)
        {
            WriteToStreamFunc(s, [&snapshotable, &v](UniqueStream& s) { snapshotable.WriteSnapshot(s, v); });
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
