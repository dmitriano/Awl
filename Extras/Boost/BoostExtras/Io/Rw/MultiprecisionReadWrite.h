/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "BoostExtras/MultiprecisionTraits.h"

#include "Awl/Io/Rw/ReadRaw.h"
#include "Awl/Io/Rw/VectorReadWrite.h"

#include <boost/multiprecision/cpp_int.hpp>
#include <boost/container/small_vector.hpp>

namespace awl::io
{
    namespace detail
    {
        inline constexpr std::uint8_t signMask = 0x80;
    }
    
    template <class Stream, class Backend, boost::multiprecision::expression_template_option ExpressionTemplates, class Context = FakeContext>
    void Read(Stream & s, boost::multiprecision::number<Backend, ExpressionTemplates>& val, const Context & ctx = {})
    {
        using Number = boost::multiprecision::number<Backend, ExpressionTemplates>;

        std::uint8_t size;
        Read(s, size, ctx);

        const bool negative = size & detail::signMask;

        size &= ~detail::signMask;

        //Floating point number may require variable size buffer, so we do not use boost::container::static_vector.
        boost::container::small_vector<std::uint8_t, awl::helpers::multiprecision_descriptor<Number>::size> v(size);

        //We read exactly size bytes here.
        ReadVector(s, v, ctx);

        import_bits(val, v.begin(), v.end());

        if (negative)
        {
            val.backend().negate();
        }
    }

    template <class Stream, class Backend, boost::multiprecision::expression_template_option ExpressionTemplates, class Context = FakeContext>
    void Write(Stream & s, const boost::multiprecision::number<Backend, ExpressionTemplates>& val, const Context & ctx = {})
    {
        using Number = boost::multiprecision::number<Backend, ExpressionTemplates>;

        boost::container::small_vector<std::uint8_t, awl::helpers::multiprecision_descriptor<Number>::size> v;

        export_bits(val, std::back_inserter(v), 8);

        //The size of bmp::int1024_t vector can be 128, so 1 byte is not enough for size + sign.
        if (v.size() > (std::numeric_limits<std::uint8_t>::max() & ~detail::signMask))
        {
            throw CorruptionException();
        }

        std::uint8_t size = static_cast<std::uint8_t>(v.size());
        
        if (val < 0)
        {
            size |= detail::signMask;
        }
        
        Write(s, size, ctx);

        WriteVector(s, v, ctx);
    }
}
