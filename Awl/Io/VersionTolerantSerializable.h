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

#include <utility>

namespace awl::io
{
    template <class T, class IStream = SequentialInputStream, class OStream = SequentialOutputStream,
        bool atomic = true, class V = mp::variant_from_struct<T>, class Context = FakeContext>
    class VersionTolerantSerializable : public Serializable<IStream, OStream>
    {
    private:

        using Reader = awl::io::Reader<V, IStream, Context>;
        using Writer = awl::io::Writer<V, OStream>;

    public:

        VersionTolerantSerializable(T& val, Context read_context = {}) :
            _val(val), _readContext(std::move(read_context)) {}

        void read(IStream& in) override
        {
            if constexpr (atomic)
            {
                if constexpr (std::is_move_assignable_v<T>)
                {
                    // Initialize newly added fields with default values.
                    T val = {};

                    read(in, val);

                    //If Read throws _val does not change.
                    _val = std::move(val);
                }
                else
                {
                    // For example, structures containing std::atomic are not copyable or movable.

                    std::vector<std::byte> v;

                    {
                        MeasureStream measure_out;

                        io::write(measure_out, _val);

                        v.reserve(measure_out.length());
                    }

                    // Save old value with a plain serialization.
                    writeSnapshot(v);

                    try
                    {
                        read(in, _val);
                    }
                    catch (const IoException&)
                    {
                        // Restore old value.
                        readSnapshot(v);

                        throw;
                    }
                }
            }
            else
            {
                read(in, _val);
            }
        }

        void write(OStream& out) const override
        {
            Writer ctx;

            ctx.writeNewPrototypes(out);
            ctx.writeV(out, _val);
        }

    protected:

        void writeSnapshot(std::vector<std::byte>& v) noexcept
        {
            VectorOutputStream v_out(v);

            io::write(v_out, _val);
        }

        void readSnapshot(const std::vector<std::byte>& v) noexcept
        {
            VectorInputStream v_in(v);

            io::read(v_in, _val);
        }

        void read(IStream& in, T& val)
        {
            Reader ctx(_readContext);
            ctx.readOldPrototypes(in);

            ctx.readV(in, val);
        }

        T& _val;

    private:

        [[no_unique_address]] Context _readContext;
    };
}
