#pragma once

#include "Awl/Io/EuphoricallySerializable.h"
#include "Awl/Io/IoException.h"
#include "Awl/StringFormat.h"

namespace awl::io
{
    struct Header
    {
        std::string format;

        size_t version;

        AWL_REFLECT(format, version);
    };

    template <class T, class V, class IStream = SequentialInputStream, class OStream = SequentialOutputStream,
        class Hash = awl::crypto::Crc64>
    class HeaderedSerializable : public EuphoricallySerializable<T, V, IStream, OStream, Hash>
    {
    private:

        using Base = EuphoricallySerializable<T, V, IStream, OStream, Hash>;

    public:

        HeaderedSerializable(Header header, T& val, size_t block_size = defaultBlockSize, 
            Hash hash = {}, size_t format_name_limit = 64u)
        :
            Base(val, block_size, std::move(hash)),
            expectedHeader(std::move(header)),
            formatNameLimit(format_name_limit)
        {}

    protected:

        bool ReadHeader(Base::HashIStream& in) override
        {
            Header actual_header;

            Read(in, actual_header, LimitedContext{ formatNameLimit });

            if (actual_header.format != expectedHeader.format)
            {
                throw IoError(awl::format() << _T("Wrong format. Expected: " << expectedHeader.format << ". Actual: " << actual_header.format << "."));
            }

            if (actual_header.version > expectedHeader.version)
            {
                throw IoError(awl::format() << _T("The version is greater then expected. Expected: " << expectedHeader.version << ". Actual: " <<
                    actual_header.version << "."));
            }

            if (actual_header.version < expectedHeader.version)
            {
                ReadOldVersion(actual_header.version);

                return false;
            }

            return true;
        }

        void WriteHeader(Base::HashOStream& out) const override
        {
            Write(out, expectedHeader, LimitedContext{ formatNameLimit });
        }

        virtual void ReadOldVersion(size_t version)
        {
            throw IoError(awl::format() << _T("Wrong version. Expected: " << expectedHeader.version << ". Actual: " << version << "."));
        }

    private:

        Header expectedHeader;

        const size_t formatNameLimit;
    };
}
