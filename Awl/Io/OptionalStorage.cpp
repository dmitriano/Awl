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
        m_logger.warning(_T("Application settings have not been loaded, from '{}' and '{}', leaving default values. Error message: {}"),
            file_name, backup_name, e.What());
    }

    return existing;
}
