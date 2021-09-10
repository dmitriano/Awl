/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Crypto/BasicHash.h"

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
            
            //typename BasicHash<N>::value_type 
            template <class InputIt>
            typename std::enable_if<sizeof(typename std::iterator_traits<InputIt>::value_type) == 1, typename BasicHash<N>::value_type>::type operator()(InputIt begin, InputIt end) const
            {
                BasicHash<N>::value_type digest;

                func(&(*begin), end - begin, digest.data());

                return digest;
            }
        };

        using Md5 = OpenSslHash<MD5, MD5_DIGEST_LENGTH>;
        using Sha1 = OpenSslHash<SHA1, SHA_DIGEST_LENGTH>;
        using Sha256 = OpenSslHash<SHA256, SHA256_DIGEST_LENGTH>;
        using Sha512 = OpenSslHash<SHA512, SHA512_DIGEST_LENGTH>;
    }
}
