/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Io/AtomicStorage.h"
#include "Awl/StringFormat.h"

using namespace awl::io;

bool AtomicStorage::Load(Value& val, const awl::String& file_name, const awl::String& backup_name)
{
    bool backup_success;
    std::tie(m_backup, backup_success) = LoadFromFile(val, backup_name, Logger::Level::Debug);

    if (backup_success)
    {
        m_logger.warning(format() << _T("The settings have been loaded from backup file '") << backup_name << _T("'."));

        m_s = awl::io::CreateUniqueFile(file_name);

        WriteToStream(m_s, val);
    }

    ClearBackup();

    if (backup_success)
    {
        return true;
    }

    //No need to reset the data after unsuccessful read because Serializable::Read should do std::mvoe in its implementation.

    bool master_success;
    std::tie(m_s, master_success) = LoadFromFile(val, file_name, Logger::Level::Warning);

    return master_success;
}

void AtomicStorage::Save(const Value& val)
{
    WriteToStream(m_backup, val);

    WriteToStream(m_s, val);
    
    ClearBackup();
}

std::tuple<awl::io::UniqueStream, bool> AtomicStorage::LoadFromFile(Value& val, const awl::String& file_name, Logger::Level level)
{
    bool success = false;

    awl::io::UniqueStream s;

    try
    {
        s = awl::io::CreateUniqueFile(file_name);

        ReadFromStream(s, val);
        
        if (s.End())
        {
            success = true;
        }
        else
        {
            m_logger.log(level, _T("Some data at the end of the settings file remained unread."));
        }
    }
    catch (const awl::io::CorruptionException&)
    {
        m_logger.log(level, (format() << _T("Corrupted settings file '") << file_name << _T("'.")));
    }
    catch (const awl::io::EndOfFileException&)
    {
        m_logger.log(level, (format() << _T("Unexpected end of settings file '") << file_name << _T("'.")));
    }
    catch (const awl::io::TypeMismatchException& e)
    {
        m_logger.log(level, (format() << _T("Type mismatch error ") << e.What() << _T(" in the settings file '") << file_name <<
            _T("' Did you include all the types including those that were removed ? .")));
    }
    catch (const awl::io::IoException& e)
    {
        m_logger.log(level, (format() << _T("General IO exception in '") << file_name <<
            _T("': ") << e.What()));
    }

    return std::make_tuple(std::move(s), success);
}
