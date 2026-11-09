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

    template <class T, class IStream = SequentialInputStream, class OStream = SequentialOutputStream,
        class Hash = awl::crypto::Crc64, class V = mp::variant_from_struct<T>>
    class HeaderedSerializable : public EuphoricallySerializable<T, IStream, OStream, Hash, V>
    {
    private:

        using Base = EuphoricallySerializable<T, IStream, OStream, Hash, V>;

    public:

        // Default budget: length + version + 64 format bytes + 64 bytes of string storage.
        HeaderedSerializable(Header header, T& val, size_t block_size = defaultBlockSize, 
            Hash hash = {}, size_t header_limit = 2 * sizeof(size_t) + 2 * 64u)
        :
            Base(val, block_size, std::move(hash)),
            expectedHeader(std::move(header)),
            headerLimit(header_limit)
        {}

    protected:

        bool readHeader(awl::io::SequentialInputStream& in) override
        {
            Header actual_header;

            read(in, actual_header, LimitedContext{ headerLimit });

            return validateHeader(in, actual_header);
        }

        bool validateHeader(awl::io::SequentialInputStream& in, const Header& actual_header)
        {
            if (actual_header.format != expectedHeader.format)
            {
                throw IoError(std::format(_T("Wrong format. Expected: {}. Actual: {}."), expectedHeader.format, actual_header.format));
            }

            if (actual_header.version > expectedHeader.version)
            {
                throw IoError(std::format(_T("The version is greater then expected. Expected: {}. Actual: {}."),
                    expectedHeader.version, actual_header.version));
            }

            if (actual_header.version < expectedHeader.version)
            {
                readOldVersion(in, actual_header.version);

                return false;
            }

            return true;
        }

        void writeHeader(awl::io::SequentialOutputStream& out) const override
        {
            write(out, expectedHeader);
        }

        virtual void readOldVersion(awl::io::SequentialInputStream& in, size_t version)
        {
            static_cast<void>(in);

            throw IoError(std::format(_T("Wrong version. Expected: {}. Actual: {}."), expectedHeader.version, version));
        }

    private:

        Header expectedHeader;

        const size_t headerLimit;
    };
}
