/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/Serializable.h"
#include "Awl/Io/Vts.h"
#include "Awl/Io/VectorStream.h"
#include "Awl/Io/MeasureStream.h"
#include "Awl/Io/IoException.h"
#include "Awl/Mp/Mp.h"

namespace awl::io
{
    template <class T, class V, class IStream = SequentialInputStream, class OStream = SequentialOutputStream, bool atomic = true>
    class VersionTolerantSerializable : public Serializable<IStream, OStream>
    {
    private:

        using Reader = awl::io::Reader<V, IStream>;
        using Writer = awl::io::Writer<V, OStream>;

    public:

        VersionTolerantSerializable(T& val) : m_val(val) {}

        void Read(IStream& in) override
        {
            if constexpr (atomic)
            {
                if constexpr (std::is_move_assignable_v<T>)
                {
                    // Initialize newly added fields with default values.
                    T val = {};

                    Read(in, val);

                    //If Read throws m_val does not change.
                    m_val = std::move(val);
                }
                else
                {
                    // For example, structures containing std::atomic are not copyable or movable.

                    std::vector<uint8_t> v;

                    {
                        MeasureStream measure_out;

                        io::Write(measure_out, m_val);

                        v.reserve(measure_out.GetLength());
                    }

                    // Save old value with a plain serialization.
                    WriteSnapshot(v);

                    try
                    {
                        Read(in, m_val);
                    }
                    catch (const IoException&)
                    {
                        // Restore old value.
                        ReadSnapshot(v);

                        throw;
                    }
                }
            }
            else
            {
                Read(in, m_val);
            }
        }

        void Write(OStream& out) const override
        {
            Writer ctx;

            ctx.WriteNewPrototypes(out);
            ctx.WriteV(out, m_val);
        }

    protected:

        void WriteSnapshot(std::vector<uint8_t>& v) noexcept
        {
            VectorOutputStream v_out(v);

            io::Write(v_out, m_val);
        }

        void ReadSnapshot(const std::vector<uint8_t>& v) noexcept
        {
            VectorInputStream v_in(v);

            io::Read(v_in, m_val);
        }

        void Read(IStream& in, T& val)
        {
            Reader ctx;
            ctx.ReadOldPrototypes(in);

            ctx.ReadV(in, val);
        }

        T& m_val;
    };
}
