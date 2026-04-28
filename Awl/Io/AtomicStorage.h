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
#include <future>
#include <cassert>
#include <memory>

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

        explicit AtomicStorage(std::shared_ptr<Logger> logger) : m_logger(std::move(logger))
        {
            assert(m_logger != nullptr);
        }

        AtomicStorage(std::shared_ptr<Logger> logger, const awl::String& file_name, const awl::String& backup_name) :
            AtomicStorage(std::move(logger))
        {
            open(file_name, backup_name);
        }

        AtomicStorage(const AtomicStorage&) = delete;
        AtomicStorage(AtomicStorage&&) = default;

        AtomicStorage& operator = (const AtomicStorage&) = delete;

        AtomicStorage& operator = (AtomicStorage&& other) noexcept
        {
            // We can't move m_saveFuture, because it holds this pointer.
            wait();

            m_s = std::move(other.m_s);
            m_backup = std::move(other.m_backup);
            return *this;
        }

        bool isEmpty() const
        {
            assert(isOpened());
            assert(!m_saveFuture.valid());

            return m_s.length() == 0 && m_backup.length() == 0;
        }

        bool isOpened() const
        {
            return m_s != UniqueStream{};
        }

        bool open(const awl::String& file_name, const awl::String& backup_name)
        {
            try
            {
                m_s = awl::io::createUniqueFile(file_name);
                const bool master_existed = openedExisting();

                m_backup = awl::io::createUniqueFile(backup_name);
                const bool backup_existed = openedExisting();

                return master_existed || backup_existed;
            }
            catch (const IoException&)
            {
                close();

                throw;
            }
        }

        bool load(Value& val);

        void save(const Value& val);

        void startSave(const Value& val);

        void startSaveLocked(const Value& val, IMutex& mutex);

        void wait()
        {
            if (m_saveFuture.valid())
            {
                m_saveFuture.get();
            }
        }

        void close()
        {
            wait();
            
            m_s = {};
            m_backup = {};
        }

    private:

        static void readFromStream(UniqueStream& s, Value& val)
        {
            s.seek(0);

            val.read(s);
        }

        template <class Func>
        static void writeToStreamFunc(UniqueStream& s, Func&& func)
        {
            s.seek(0);

            func(s);

            s.truncate();
            s.flush();
        }

        static void writeToStream(UniqueStream& s, const Value& val)
        {
            writeToStreamFunc(s, std::bind(&Value::write, &val, std::ref(s)));
        }

        static void writeSnapshot(UniqueStream& s, std::shared_ptr<Snapshot> snapshot)
        {
            writeToStreamFunc(s, std::bind(&Snapshot::write, snapshot, std::ref(s)));
        }

        void writeToStreamAndClearBackup(const Value& val)
        {
            writeToStream(m_backup, val);

            writeToStream(m_s, val);

            clearBackup();
        }

        void writeSnapshotsAndClearBackup(std::shared_ptr<Snapshot> snapshot)
        {
            writeSnapshot(m_backup, snapshot);

            writeSnapshot(m_s, snapshot);

            clearBackup();
        }

        bool loadFromFile(Value& val, awl::io::UniqueStream& s, std::string level);

        void clearBackup()
        {
            m_backup.seek(0);
            m_backup.truncate();
            m_backup.flush();
        }

        std::shared_ptr<Logger> m_logger;

        UniqueStream m_s;
        UniqueStream m_backup;

        std::future<void> m_saveFuture;
    };
}
