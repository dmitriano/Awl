#include "PersistentObject.h"

#include "Awl/StringFormat.h"
#include "Awl/Io/IoException.h"

namespace awl::io::helpers
{
    bool LoadFromStorage(Serializable<>& val, AtomicStorage& storage, awl::Logger& logger, const String& file_name, const String& backup_name, bool allow_default)
    {
        bool loaded = false;

        try
        {
            // Throws if can't create files.
            if (storage.Open(file_name, backup_name))
            {
                loaded = storage.Load(val);
            }
        }
        catch (const IoException&)
        {
            if (!allow_default)
            {
                std::throw_with_nested(IoError(awl::format() << _T("Failed to open settings files '") <<
                    file_name << _T("' and '") << backup_name << "'"));
            }
        }

        if (!loaded)
        {
            if (!allow_default)
            {
                throw awl::io::IoError(awl::format() << _T("Failed to load settings from '") <<
                    file_name << _T("' and '") << backup_name << "'");
            }

            logger.warning(awl::format() << _T("Settings have not been loaded, from ") <<
                file_name << _T("' and '") << backup_name <<
                _T(", leaving default values."));
        }

        return loaded;
    }
}

