#pragma once

#include "Awl/Crypto/BasicHash.h"
#include "Awl/Io/HashStream.h"

namespace awl 
{
    namespace io
    {
        using BufferedInputStream = HashInputStream<crypto::FakeHash>;
        using BufferedOutputStream = HashOutputStream<crypto::FakeHash>;
    }
}
