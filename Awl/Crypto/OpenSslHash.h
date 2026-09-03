/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Crypto/BasicHash.h"
#include "Awl/DataCast.h"

#include <type_traits>
#include <openssl/md5.h>
#include <openssl/sha.h>

namespace awl
{
    namespace crypto
    {
        template <unsigned char * (*func)(const unsigned char *, size_t, unsigned char *), size_t N>
        class OpenSslHash : public BasicHash<N>
        {
        public:
            
            template <class T>
                requires (sizeof(T) == 1)
            typename BasicHash<N>::value_type operator()(const T* begin, const T* end) const
            {
                typename BasicHash<N>::value_type digest;

                func(
                    reinterpret_cast<const unsigned char*>(begin),
                    end - begin,
                    reinterpret_cast<unsigned char*>(digest.data()));

                return digest;
            }
        };

        using Md5 = OpenSslHash<MD5, MD5_DIGEST_LENGTH>;
        using Sha1 = OpenSslHash<SHA1, SHA_DIGEST_LENGTH>;
        using Sha256 = OpenSslHash<SHA256, SHA256_DIGEST_LENGTH>;
        using Sha512 = OpenSslHash<SHA512, SHA512_DIGEST_LENGTH>;
    }
}
