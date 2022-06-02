/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/Rw/ReadRaw.h"
#include "Awl/Io/Rw/VectorReadWrite.h"

#include <boost/multiprecision/cpp_int.hpp>

namespace awl::io
{
    namespace detail
    {
        inline constexpr std::uint8_t signMask = 0x80; // static_cast<std::uint8_t>(1) << (sizeof(std::uint8_t) - 1u);
    }
    
    template <class Stream, class Backend, boost::multiprecision::expression_template_option ExpressionTemplates, class Context = FakeContext>
    void Read(Stream & s, boost::multiprecision::number<Backend, ExpressionTemplates>& val, const Context & ctx = {})
    {
        std::uint8_t size;
        Read(s, size, ctx);

        const bool negative = size & detail::signMask;

        size &= ~detail::signMask;

        std::vector<std::uint8_t> v(size);
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
        std::vector<std::uint8_t> v;

        export_bits(val, std::back_inserter(v), 8);

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
