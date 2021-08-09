#pragma once

#include "Awl/StringFormat.h"
#include "Awl/Io/Serializer.h"
#include "Awl/Io/HashStream.h"
#include "Awl/Io/MpHelpers.h"
#include "Awl/Crypto/Crc64.h"

#include "Awl/Io/NativeStream.h"

namespace awl
{
    template <class T, class V, class Logger, class Format>
    class SettingsStorage
    {
    private:

        using Hash = awl::crypto::Crc64;

        using HashInputStream = io::HashInputStream<Hash, io::UniqueStream>;
        using HashOutputStream = io::HashOutputStream<Hash, io::UniqueStream>;
        
        using Reader = io::Reader<V, HashInputStream>;
        using Writer = io::Writer<V, HashOutputStream>;

    public:

        SettingsStorage(T & val, Logger& logger) : m_logger(logger), m_val(val)
        {
        }

        bool Load(const String& file_name, const String& backup_name)
        {
            bool backup_success;
            std::tie(m_backup, backup_success) = LoadFromFile(backup_name);
            
            if (backup_success)
            {
                m_s = io::CreateUniqueFile(file_name);

                WriteToStream(m_s);
            }

            ClearBackup();

            if (backup_success)
            {
                return true;
            }

            //Reset the date after unsuccessful read.
            m_val = {};

            bool master_success;
            std::tie(m_s, master_success) = LoadFromFile(file_name);

            return master_success;
        }

        void Save()
        {
            WriteToStream(m_backup);

            WriteToStream(m_s);

            ClearBackup();
        }

    private:

        bool ReadFromStream(io::UniqueStream& s)
        {
            HashInputStream in(s);

            Reader ctx;
            ctx.ReadOldPrototypes(in);

            ctx.ReadV(in, m_val);

            return in.End();
        }

        void WriteToStream(io::UniqueStream& s)
        {
            s.Seek(0);

            {
                HashOutputStream out(s);

                Writer ctx;

                ctx.WriteNewPrototypes(out);
                ctx.WriteV(out, m_val);
            }

            s.Truncate();
            s.Flush();
        }

        std::tuple<io::UniqueStream, bool> LoadFromFile(const String& file_name)
        {
            bool success = false;

            io::UniqueStream s;
            
            try
            {
                s = io::CreateUniqueFile(file_name);

                if (ReadFromStream(s))
                {
                    success = true;
                }
                else
                {
                    m_logger.warning("Some data at the end of the settings file remained unread.");
                }
            }
            catch (const io::NativeException& e)
            {
                m_logger.warning((Format() << _T("Can't open settings file '") << file_name << 
                    _T("' Native error: ") << e.GetMessage()));
            }
            catch (const io::CorruptionException&)
            {
                m_logger.warning((Format() << _T("Corrupted settings file '") << file_name << _T("'.")));
            }
            catch (const io::EndOfFileException&)
            {
                m_logger.warning((Format() << _T("Unexpected end of settings file '") << file_name << _T("'.")));
            }
            catch (const io::TypeMismatchException& e)
            {
                m_logger.warning((Format() << _T("Type mismatch error ") << e.GetMessage() << _T(" in the settings file '") << file_name <<
                    _T("' Did you include all the types including those that were removed ? .")));
            }

            return std::make_tuple(std::move(s), success);
        }

        void ClearBackup()
        {
            m_backup.Seek(0);
            m_backup.Truncate();
            m_backup.Flush();
        }

        Logger& m_logger;
        
        T& m_val;
        
        io::UniqueStream m_s;
        io::UniqueStream m_backup;
    };
}
