#pragma once

#include "Awl/Crypto/BasicHash.h"
#include "Awl/Io/HashStream.h"

namespace awl 
{
    namespace io
    {
        typedef HashInputStream<crypto::FakeHash> BufferedInputStream;
        typedef HashOutputStream<crypto::FakeHash> BufferedOutputStream;
    }
}
