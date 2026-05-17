/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Io/AtomicStorage.h"
#include "Awl/StringFormat.h"
#include "Awl/OptionalMutex.h"

#include <mutex>

using namespace awl::io;

bool AtomicStorage::load(Value& val)
{
    wait();

    bool backup_success = loadFromFile(val, _backup, LogLevel::Debug);

    if (backup_success)
    {
        _logger->warning(_T("The settings have been loaded from backup file '{}'."), _backup.fileName());

        writeToStream(_s, val);
    }

    clearBackup();

    if (backup_success)
    {
        return true;
    }

    //No need to reset the data after unsuccessful read because Serializable::read should do std::mvoe in its implementation.

    bool master_success = loadFromFile(val, _s, LogLevel::Warning);

    return master_success;
}

void AtomicStorage::save(const Value& val)
{
    wait();
    
    writeToStreamAndClearBackup(val);
}

void AtomicStorage::startSave(const Value& val)
{
    awl::FakeMutex fm;

    startSaveLocked(val, fm);
}

void AtomicStorage::startSaveLocked(const Value& val, IMutex& mutex)
{
    std::unique_lock lock(mutex);

    // For example, val is updated on a render thread in a video game.
    const awl::io::Snapshotable& snapshotable = dynamic_cast<const awl::io::Snapshotable&>(val);

    std::shared_ptr<Snapshot> snapshot = snapshotable.makeShanshot();

    lock.unlock();

    wait();

    _saveFuture = std::async(std::launch::async, std::bind(&AtomicStorage::writeSnapshotsAndClearBackup, this, std::move(snapshot)));
}

bool AtomicStorage::loadFromFile(Value& val, awl::io::UniqueStream& s, const std::string level)
{
    bool success = false;

    try
    {
        readFromStream(s, val);
        
        if (s.end())
        {
            success = true;
        }
        else
        {
            _logger->log(level, "Some data at the end of the settings file remained unread.");
        }
    }
    catch (const awl::io::CorruptionException&)
    {
        _logger->log(level, std::format(_T("Corrupted settings file '{}'."), s.fileName()));
    }
    catch (const awl::io::EndOfFileException&)
    {
        _logger->log(level, std::format(_T("Unexpected end of settings file '{}'."), s.fileName()));
    }
    catch (const awl::io::TypeMismatchException& e)
    {
        _logger->log(level, std::format(_T("Type mismatch error {} in the settings file '{}' Did you include all the types including those that were removed ? ."),
            e.message(), s.fileName()));
    }
    catch (const awl::io::IoException& e)
    {
        _logger->log(level, std::format(_T("General IO exception in '{}': {}"), s.fileName(), e.message()));
    }

    return success;
}
