/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Io/OptionalStorage.h"
#include "Awl/Io/IoException.h"
#include "Awl/StringFormat.h"

using namespace awl::io;

bool OptionalStorage::Open(const awl::String& file_name, const awl::String& backup_name)
{
    bool existing = false;

    try
    {
        existing = m_storage.Open(file_name, backup_name);
    }
    catch (const IoException& e)
    {
        m_logger.warning(awl::format() << _T("Settings have not been loaded, from ") <<
            file_name << _T("' and '") << backup_name <<
            _T(", leaving default values.") <<
            _T("Error message: ") << e.What());
    }

    return existing;
}

void OptionalStorage::Save(const Value& val, IMutex* p_mutex)
{
    if (m_storage.IsOpened())
    {
        try
        {
            return m_storage.Save(val, p_mutex);
        }
        catch (const IoException& e)
        {
            m_logger.warning(awl::format() << _T("Settings were not saved correctly. ") <<
                _T("Error message: ") << e.What());
        }
    }
}
