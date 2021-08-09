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

        using Reader = io::Reader<V, io::HashInputStream<Hash>>;
        using Writer = io::Writer<V, io::HashOutputStream<Hash>>;

    public:

        SettingsStorage(T & val, Logger& logger) : m_logger(logger), m_val(val)
        {
        }

        bool Load(const String& file_name)
        {
            bool success = false;

            try
            {
                m_s = io::CreateUniqueFile(file_name);

                io::HashInputStream<Hash> in(m_s);

                Reader ctx;
                ctx.ReadOldPrototypes(in);

                ctx.ReadV(in, m_val);

                if (in.End())
                {
                    success = true;
                }
                else
                {
                    m_logger.warning("Some data at the end of the settings file remained unread.");
                }
            }
            catch (const io::NativeException&)
            {
                m_logger.warning((Format() << _T("Can't open settings file '") << file_name << _T("'.")));
            }
            catch (const io::CorruptionException&)
            {
                m_logger.warning("Corrupted settings file.");
            }
            catch (const io::EndOfFileException&)
            {
                m_logger.warning("Corrupted settings file (probably truncated).");
            }
            catch (const io::TypeMismatchException&)
            {
                m_logger.warning("Type mismatch error in the settings file. Did you include all the types including those that were removed?");
            }

            return success;
        }

        void Save()
        {
            {
                m_s.Seek(0);

                io::HashOutputStream<Hash> out(m_s);

                Writer ctx;

                ctx.WriteNewPrototypes(out);
                ctx.WriteV(out, m_val);
            }

            m_s.Truncate();
            m_s.Flush();
        }

    private:

        Logger& m_logger;
        
        T& m_val;
        
        io::UniqueStream m_s;
    };
}
