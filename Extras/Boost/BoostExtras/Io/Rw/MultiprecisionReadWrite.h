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
    template <class Stream, class Backend, boost::multiprecision::expression_template_option ExpressionTemplates, class Context = FakeContext>
    void Read(Stream & s, boost::multiprecision::number<Backend, ExpressionTemplates>& val, const Context & ctx = {})
    {
        std::uint8_t size;
        Read(s, size, ctx);

        std::vector<std::uint8_t> v(size);
        ReadVector(s, v, ctx);

        import_bits(val, v.begin(), v.end());
    }

    template <class Stream, class Backend, boost::multiprecision::expression_template_option ExpressionTemplates, class Context = FakeContext>
    void Write(Stream & s, const boost::multiprecision::number<Backend, ExpressionTemplates>& val, const Context & ctx = {})
    {
        std::vector<std::uint8_t> v;

        export_bits(val, std::back_inserter(v), 8);

        if (v.size() > std::numeric_limits<std::uint8_t>::max())
        {
            throw CorruptionException();
        }

        const std::uint8_t size = static_cast<std::uint8_t>(v.size());
        Write(s, size, ctx);

        WriteVector(s, v, ctx);
    }
}
