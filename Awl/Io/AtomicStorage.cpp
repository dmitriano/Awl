/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Io/AtomicStorage.h"
#include "Awl/StringFormat.h"

using namespace awl::io;

bool AtomicStorage::Load(Value& val)
{
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
    WriteToStream(m_backup, val);

    WriteToStream(m_s, val);
    
    ClearBackup();
}

bool AtomicStorage::LoadFromFile(Value& val, awl::io::UniqueStream& s, LogLevel level)
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
