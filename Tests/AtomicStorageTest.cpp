/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Testing/UnitTest.h"
#include "Awl/Io/HashingSerializable.h"
#include "Awl/Io/PlainSerializable.h"
#include "Awl/Io/VersionTolerantSerializable.h"
#include "Awl/Io/AtomicStorage.h"
#include "Awl/ConsoleLogger.h"
#include "VtsData.h"

using namespace awl::testing;
using namespace awl::testing::helpers;

namespace
{
    awl::Char master_name[] = _T("atomic_storage.dat");
    awl::Char backup_name[] = _T("atomic_storage.bak");
}

AWT_TEST(AtomicStorage)
{
    AWT_UNUSED_CONTEXT;
    
    awl::ConsoleLogger logger;

    using Value1 = awl::io::VersionTolerantSerializable<v1::B, V1, awl::io::UniqueStream, awl::io::UniqueStream>;
    
    {
        {
            v1::B b = v1::b_expected;
            Value1 val(b);
            awl::io::AtomicStorage storage(logger);
            AWT_ASSERT(!storage.Load(val, master_name, backup_name));
            AWT_ASSERT(b == v1::b_expected);
            storage.Save(val);
        }

        {
            v1::B b;
            Value1 val(b);
            awl::io::AtomicStorage storage(logger);
            AWT_ASSERT(storage.Load(val, master_name, backup_name));
            AWT_ASSERT(b == v1::b_expected);
        }
    }

    {
        v2::B b = v2::b_expected;
        awl::io::VersionTolerantSerializable<v2::B, V2, awl::io::UniqueStream, awl::io::UniqueStream> val(b);

        {
            awl::io::AtomicStorage storage(logger);
            AWT_ASSERT(storage.Load(val, master_name, backup_name));
            AWT_ASSERT(b == v2::b_expected);
        }
    }
}
