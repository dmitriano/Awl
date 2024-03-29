/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Testing/UnitTest.h"
#include "Awl/Io/HashingSerializable.h"
#include "Awl/Io/PlainSerializable.h"
#include "Awl/Io/VersionTolerantSerializable.h"
#include "Awl/Io/EuphoricallySerializable.h"
#include "Awl/Io/AtomicStorage.h"
#include "Awl/Io/NativeStream.h"
#include "Awl/ConsoleLogger.h"
#include "Awl/ScopeGuard.h"
#include "Awl/Random.h"
#include "VtsData.h"

#include <filesystem>
namespace fs = std::filesystem;

using namespace awl::testing;
using namespace awl::testing::helpers;

namespace
{
    awl::Char master_name[] = _T("atomic_storage.dat");
    awl::Char backup_name[] = _T("atomic_storage.bak");
    awl::Char temp_name[] = _T("atomic_storage.tmp");

    void RemoveFiles()
    {
        fs::remove(master_name);
        fs::remove(backup_name);
    }

    void CorruptFile(awl::Char* file_name)
    {
        awl::io::UniqueStream s = awl::io::CreateUniqueFile(file_name);

        auto len = s.GetLength();

        AWT_ASSERT(len != 0);

        std::uniform_int_distribution<size_t> dist(1, len - 1);

        const size_t i = dist(awl::random());

        s.Seek(i);
        uint8_t bad;
        s.Read(&bad, 1);

        s.Seek(i);
        ++bad;
        s.Write(&bad, 1);
        s.Flush();
    }

    void SwapFiles()
    {
        fs::rename(master_name, temp_name);
        fs::rename(backup_name, master_name);
        fs::rename(temp_name, backup_name);
    }

    void DuplicateMaserFile()
    {
        fs::remove(backup_name);
        fs::copy(master_name, backup_name);
    }

    using Hash = awl::crypto::Crc64;
    using HashInputStream = awl::io::HashInputStream<Hash, awl::io::UniqueStream>;
    using HashOutputStream = awl::io::HashOutputStream<Hash, awl::io::UniqueStream>;
    using HashingSerializable = awl::io::HashingSerializable<awl::io::UniqueStream, awl::io::UniqueStream, Hash>;

    using Value1 = awl::io::VersionTolerantSerializable<v1::B, V1, HashInputStream, HashOutputStream>;
    using Value2 = awl::io::VersionTolerantSerializable<v2::B, V2, HashInputStream, HashOutputStream>;

    template <class Value>
    bool LoadValue(awl::io::AtomicStorage& storage, Value& val, bool existed = true)
    {
        AWT_ASSERT(storage.Open(master_name, backup_name) == existed);
        return storage.Load(val);
    }
}

AWT_TEST(AtomicStoragePlain)
{
    auto guard = awl::make_scope_guard(RemoveFiles);

    awl::ConsoleLogger logger(context.out);

    using Value = awl::io::PlainSerializable<v2::B, HashInputStream, HashOutputStream>;

    {
        v2::B b = v2::b_expected;
        Value val(b);
        HashingSerializable hashed_val(val);
        awl::io::AtomicStorage storage(logger);
        AWT_ASSERT(!LoadValue(storage, hashed_val, false));
        AWT_ASSERT(b == v2::b_expected);
        storage.Save(hashed_val);
    }

    {
        v2::B b;
        Value val(b);
        HashingSerializable hashed_val(val);
        awl::io::AtomicStorage storage(logger);
        AWT_ASSERT(LoadValue(storage, hashed_val));
        AWT_ASSERT(b == v2::b_expected);
    }

    CorruptFile(master_name);

    {
        v2::B b;
        const v2::B saved_b = b;
        Value val(b);
        HashingSerializable hashed_val(val);
        awl::io::AtomicStorage storage(logger);
        AWT_ASSERT(!LoadValue(storage, hashed_val));
        AWT_ASSERT(b == saved_b);

        b = v2::b_expected;
        storage.Save(hashed_val);
    }

    auto load = [&logger]()
    {
        v2::B b;
        Value val(b);
        HashingSerializable hashed_val(val);
        awl::io::AtomicStorage storage(logger);
        AWT_ASSERT(LoadValue(storage, hashed_val));
        AWT_ASSERT(b == v2::b_expected);
    };

    load();

    SwapFiles();

    load();

    DuplicateMaserFile();
    CorruptFile(master_name);

    load();
}

AWT_TEST(AtomicStorageVts)
{
    auto guard = awl::make_scope_guard(RemoveFiles);

    awl::ConsoleLogger logger(context.out);

    {
        {
            v1::B b = v1::b_expected;
            Value1 val(b);
            HashingSerializable hashed_val(val);
            awl::io::AtomicStorage storage(logger);
            AWT_ASSERT(!storage.IsOpened());
            AWT_ASSERT(!LoadValue(storage, hashed_val, false));
            AWT_ASSERT(storage.IsOpened());
            AWT_ASSERT(b == v1::b_expected);
            storage.Save(hashed_val);
        }

        {
            v1::B b;
            Value1 val(b);
            HashingSerializable hashed_val(val);
            awl::io::AtomicStorage storage(logger);
            AWT_ASSERT(LoadValue(storage, hashed_val));
            AWT_ASSERT(b == v1::b_expected);
        }
    }

    {
        v2::B b;
        Value2 val(b);
        HashingSerializable hashed_val(val);
        awl::io::AtomicStorage storage(logger);
        AWT_ASSERT(LoadValue(storage, hashed_val));
        AWT_ASSERT(b == v2::b_expected);
    }

    CorruptFile(master_name);

    {
        v2::B b;
        const v2::B saved_b = b;
        Value2 val(b);
        HashingSerializable hashed_val(val);
        awl::io::AtomicStorage storage(logger);
        AWT_ASSERT(!LoadValue(storage, hashed_val));
        AWT_ASSERT(b == saved_b);

        b = v2::b_expected;
        storage.Save(hashed_val);
    }

    auto load = [&logger]()
    {
        v2::B b;
        Value2 val(b);
        HashingSerializable hashed_val(val);
        awl::io::AtomicStorage storage(logger);
        AWT_ASSERT(LoadValue(storage, hashed_val));
        AWT_ASSERT(b == v2::b_expected);
    };

    load();

    SwapFiles();

    load();

    DuplicateMaserFile();
    CorruptFile(master_name);

    load();
}

namespace
{
    awl::io::AtomicStorage MakeStorage(awl::Logger& logger)
    {
        return awl::io::AtomicStorage(logger, master_name, backup_name);
    }
}

AWT_TEST(AtomicStorageMove)
{
    auto guard = awl::make_scope_guard(RemoveFiles);

    awl::ConsoleLogger logger(context.out);

    awl::io::AtomicStorage storage = MakeStorage(logger);

    {
        v1::B b = v1::b_expected;
        Value1 val(b);
        HashingSerializable hashed_val(val);
        AWT_ASSERT(!storage.Load(hashed_val));
        AWT_ASSERT(b == v1::b_expected);
        storage.Save(hashed_val);
    }

    awl::io::AtomicStorage storage1 = std::move(storage);

    {
        v2::B b;
        Value2 val(b);
        HashingSerializable hashed_val(val);
        storage1.Load(hashed_val);
        AWT_ASSERT(b == v2::b_expected);
    }
}

AWT_TEST(AtomicStorageSave)
{
    auto guard = awl::make_scope_guard(RemoveFiles);

    awl::ConsoleLogger logger(context.out);

    awl::io::AtomicStorage storage = MakeStorage(logger);
    AWT_ASSERT(storage.IsEmpty());

    {
        v1::B b = v1::b_expected;
        Value1 val(b);
        HashingSerializable hashed_val(val);
        storage.Save(hashed_val);
        AWT_ASSERT(!storage.IsEmpty());
    }

    awl::io::AtomicStorage storage1 = std::move(storage);
    AWT_ASSERT(!storage1.IsEmpty());

    {
        v2::B b;
        Value2 val(b);
        HashingSerializable hashed_val(val);
        storage1.Load(hashed_val);
        AWT_ASSERT(b == v2::b_expected);
    }
}

AWT_TEST(AtomicStorageEuphorical)
{
    auto guard = awl::make_scope_guard(RemoveFiles);

    awl::ConsoleLogger logger(context.out);

    awl::io::AtomicStorage storage = MakeStorage(logger);
    AWT_ASSERT(storage.IsEmpty());

    using Value = awl::io::EuphoricallySerializable<v2::B, V2, awl::io::UniqueStream, awl::io::UniqueStream>;

    {
        v2::B b = v2::b_expected;
        Value val(b);
        storage.Save(val);
    }

    {
        v2::B b;
        Value val(b);
        storage.Load(val);
        AWT_ASSERT(b == v2::b_expected);
    }
}
