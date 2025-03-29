/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Io/AtomicStorage.h"
#include "Awl/StringFormat.h"
#include "Awl/OptionalMutex.h"

#include <mutex>

using namespace awl::io;

bool AtomicStorage::Load(Value& val)
{
    Wait();

    bool backup_success = LoadFromFile(val, m_backup, LogLevel::Debug);

    if (backup_success)
    {
        m_logger.warning(format() << _T("The settings have been loaded from backup file '") << m_backup.GetFileName() << "'.");

        WriteToStream(m_s, val);
    }

    ClearBackup();

    if (backup_success)
    {
        return true;
    }

    //No need to reset the data after unsuccessful read because Serializable::Read should do std::mvoe in its implementation.

    bool master_success = LoadFromFile(val, m_s, LogLevel::Warning);

    return master_success;
}

void AtomicStorage::Save(const Value& val)
{
    Wait();
    
    WriteToStreamAndClearBackup(val);
}

void AtomicStorage::StartSave(const Value& val)
{
    awl::FakeMutex fm;

    StartSaveLocked(val, fm);
}

void AtomicStorage::StartSaveLocked(const Value& val, IMutex& mutex)
{
    std::unique_lock lock(mutex);

    // For example, val is updated on a render thread in a video game.
    const awl::io::Snapshotable<UniqueStream>& snapshotable = dynamic_cast<const awl::io::Snapshotable<UniqueStream>&>(val);

    std::shared_ptr<Snapshot<UniqueStream>> snapshot = snapshotable.MakeShanshot();

    lock.unlock();

    Wait();

    m_saveFuture = std::async(std::launch::async, std::bind(&AtomicStorage::WriteSnapshotsAndClearBackup, this, std::move(snapshot)));
}

bool AtomicStorage::LoadFromFile(Value& val, awl::io::UniqueStream& s, std::string level)
{
    bool success = false;

    try
    {
        ReadFromStream(s, val);
        
        if (s.End())
        {
            success = true;
        }
        else
        {
            m_logger.log(level, "Some data at the end of the settings file remained unread.");
        }
    }
    catch (const awl::io::CorruptionException&)
    {
        m_logger.log(level, (format() << "Corrupted settings file '" << s.GetFileName() << "'."));
    }
    catch (const awl::io::EndOfFileException&)
    {
        m_logger.log(level, (format() << "Unexpected end of settings file '" << s.GetFileName() << "'."));
    }
    catch (const awl::io::TypeMismatchException& e)
    {
        m_logger.log(level, (format() << _T("Type mismatch error ") << e.What() << " in the settings file '" << s.GetFileName() <<
            "' Did you include all the types including those that were removed ? ."));
    }
    catch (const awl::io::IoException& e)
    {
        m_logger.log(level, (format() << "General IO exception in '" << s.GetFileName() <<
            "': " << e.What()));
    }

    return success;
}
