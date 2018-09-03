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

        typedef OpenSslHash<MD5, MD5_DIGEST_LENGTH> Md5;
        typedef OpenSslHash<SHA1, SHA_DIGEST_LENGTH> Sha1;
        typedef OpenSslHash<SHA256, SHA256_DIGEST_LENGTH> Sha256;
        typedef OpenSslHash<SHA512, SHA512_DIGEST_LENGTH> Sha512;
    }
}
