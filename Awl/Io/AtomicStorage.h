/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/ILogger.h"
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

        explicit AtomicStorage(std::shared_ptr<ILogger> logger) : _logger(std::move(logger))
        {
            assert(_logger != nullptr);
        }

        AtomicStorage(std::shared_ptr<ILogger> logger, const awl::String& file_name, const awl::String& backup_name) :
            AtomicStorage(std::move(logger))
        {
            open(file_name, backup_name);
        }

        AtomicStorage(const AtomicStorage&) = delete;
        AtomicStorage(AtomicStorage&&) = default;

        AtomicStorage& operator = (const AtomicStorage&) = delete;

        AtomicStorage& operator = (AtomicStorage&& other) noexcept
        {
            // We can't move _saveFuture, because it holds this pointer.
            wait();

            _s = std::move(other._s);
            _backup = std::move(other._backup);
            return *this;
        }

        bool isEmpty() const
        {
            assert(isOpened());
            assert(!_saveFuture.valid());

            return _s.length() == 0 && _backup.length() == 0;
        }

        bool isOpened() const
        {
            return _s != UniqueStream{};
        }

        bool open(const awl::String& file_name, const awl::String& backup_name)
        {
            try
            {
                _s = awl::io::createUniqueFile(file_name);
                const bool master_existed = openedExisting();

                _backup = awl::io::createUniqueFile(backup_name);
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
            if (_saveFuture.valid())
            {
                _saveFuture.get();
            }
        }

        void close()
        {
            wait();
            
            _s = {};
            _backup = {};
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
            writeToStream(_backup, val);

            writeToStream(_s, val);

            clearBackup();
        }

        void writeSnapshotsAndClearBackup(std::shared_ptr<Snapshot> snapshot)
        {
            writeSnapshot(_backup, snapshot);

            writeSnapshot(_s, snapshot);

            clearBackup();
        }

        bool loadFromFile(Value& val, awl::io::UniqueStream& s, std::string level);

        void clearBackup()
        {
            _backup.seek(0);
            _backup.truncate();
            _backup.flush();
        }

        std::shared_ptr<ILogger> _logger;

        UniqueStream _s;
        UniqueStream _backup;

        std::future<void> _saveFuture;
    };
}
