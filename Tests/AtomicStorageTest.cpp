/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Testing/UnitTest.h"
#include "Awl/Io/HashingSerializable.h"
#include "Awl/Io/PlainSerializable.h"
#include "Awl/Io/VersionTolerantSerializable.h"
#include "Awl/Io/EuphoricallySerializable.h"
#include "Awl/Io/HeaderedSerializable.h"
#include "Awl/Io/AtomicStorage.h"
#include "Awl/Io/NativeStream.h"
#include "Awl/Io/FieldMap.h"
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
        v2::B b = {};
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
        v2::B b = {};
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

AWT_TEST(AtomicStorageEuphorical1)
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

namespace
{
    AWL_SEQUENTIAL_ENUM(BoardType, Square, Hexagonal)
    AWL_SEQUENTIAL_ENUM(ClusterType, Line, Loop)
}

AWL_ENUM_TRAITS(, BoardType)
AWL_ENUM_TRAITS(, ClusterType)

namespace
{
    class GameParamsV1
    {
    public:

        BoardType Type;
        int GridSize;
        int ColorCount;
        int NewCount;
        int LineLength;
        bool Prompting;

        AWL_REFLECT(Type, GridSize, ColorCount, NewCount, LineLength, Prompting)
    };

    // Type was renamed with boardType and clusterType was added.
    class GameParamsV2
    {
    public:

        BoardType boardType;
        ClusterType clusterType;
        int GridSize;
        int ColorCount;
        int NewCount;
        int LineLength;
        bool Prompting;

        AWL_REFLECT(boardType, clusterType, GridSize, ColorCount, NewCount, LineLength, Prompting)

        auto as_v1_tuple() const
        {
            return std::tie(boardType, GridSize, ColorCount, NewCount, LineLength, Prompting);
        }
    };
}

namespace awl::io
{
    template <>
    class FieldMap<GameParamsV2>
    {
    public:

        static std::string_view GetNewName(std::string_view old_name)
        {
            using namespace std::literals;

            if (old_name == "Type"sv)
            {
                return "boardType"sv;
            }

            return old_name;
        }
    };
}

namespace
{
    template <class Value1, class Value2>
    void TestRenameAndDefaultFields(const awl::testing::TestContext& context)
    {
        auto guard = awl::make_scope_guard(RemoveFiles);

        awl::ConsoleLogger logger(context.out);

        awl::io::AtomicStorage storage = MakeStorage(logger);
        AWT_ASSERT(storage.IsEmpty());

        const GameParamsV1 gp1_sample = { BoardType::Hexagonal, 1, 2, 3, 4, true };

        {
            GameParamsV1 gp1 = gp1_sample;

            Value1 val(gp1);
            storage.Save(val);
        }

        {
            GameParamsV2 gp2;

            Value2 val(gp2);
            storage.Load(val);

            // Ensure clusterType is initialized into its default value.
            AWT_ASSERT(gp2.clusterType == ClusterType::Line);
            AWT_ASSERT(gp2.as_v1_tuple() == gp1_sample.as_tuple());
        }
    }
}

AWT_TEST(AtomicStorageVts2)
{
    TestRenameAndDefaultFields<
        awl::io::VersionTolerantSerializable<GameParamsV1, awl::io::helpers::variant_from_structs<GameParamsV1>,
            awl::io::UniqueStream, awl::io::UniqueStream>,
        awl::io::VersionTolerantSerializable<GameParamsV2, awl::io::helpers::variant_from_structs<GameParamsV2>,
            awl::io::UniqueStream, awl::io::UniqueStream>>
    (context);
}

AWT_TEST(AtomicStorageEuphorical2)
{
    TestRenameAndDefaultFields<
        awl::io::EuphoricallySerializable<GameParamsV1, awl::io::helpers::variant_from_structs<GameParamsV1>,
            awl::io::UniqueStream, awl::io::UniqueStream>,
        awl::io::EuphoricallySerializable<GameParamsV2, awl::io::helpers::variant_from_structs<GameParamsV2>,
            awl::io::UniqueStream, awl::io::UniqueStream>>
    (context);
}

AWT_TEST(Shapshot)
{
    using Value = awl::io::EuphoricallySerializable<v2::B, V2, awl::io::VectorInputStream, awl::io::VectorOutputStream>;

    v2::B b = v2::b_expected;
    Value val(b);

    std::vector<uint8_t> expected_v;

    {
        awl::io::VectorOutputStream out(expected_v);

        val.Write(out);
    }

    auto snapshot = val.MakeShanshot();

    std::vector<uint8_t> actual_v;

    {
        awl::io::VectorOutputStream out(actual_v);

        val.WriteSnapshot(out, snapshot);
    }

    AWT_ASSERT(actual_v == expected_v);

    context.out << _T("Snapshot size: ") << snapshot.size() << _T(" bytes") << std::endl;
    context.out << _T("Snapshot size: ") << actual_v.size() << _T(" bytes") << std::endl;
    context.out << _T("Hash size: ") << actual_v.size() - snapshot.size() << _T(" bytes") << std::endl;
}

namespace
{
    class HeaderedValue : public awl::io::HeaderedSerializable<v2::B, V2, awl::io::VectorInputStream, awl::io::VectorOutputStream>
    {
    private:

        using Base = awl::io::HeaderedSerializable<v2::B, V2, awl::io::VectorInputStream, awl::io::VectorOutputStream>;

    public:

        using Base::Base;

        virtual void ReadOldVersion(size_t version)
        {
            oldVersion = version;
        }

        static constexpr size_t NoVersion = std::numeric_limits<size_t>::max();

        size_t oldVersion = NoVersion;
    };
}

AWT_TEST(HeaderedSerializable)
{
    AWT_UNUSED_CONTEXT;

    using Value = HeaderedValue;

    // A header from the current version of the software.
    const awl::io::Header current_header{ "SAMPLE FORMAT", 2u };

    std::vector<uint8_t> v;

    {
        awl::io::VectorOutputStream out(v);

        v2::B b = v2::b_expected;
        Value val(current_header, b);
        val.Write(out);
    }

    {
        awl::io::VectorInputStream in(v);

        v2::B b;
        Value val(current_header, b);
        val.Read(in);
        AWT_ASSERT(b == v2::b_expected);
        // Check the versions match.
        AWT_ASSERT(val.oldVersion == HeaderedValue::NoVersion);
    }

    awl::testing::Assert::Throws<awl::io::IoException>([&v, &current_header]()
    {
        const awl::io::Header wrong_header{ "WRONG FORMAT", current_header.version };

        awl::io::VectorInputStream in(v);

        v2::B b;
        Value val(wrong_header, b);
        val.Read(in);
        AWT_ASSERT(b == v2::b_expected);
    });

    // Read an older version from a newer stream.
    awl::testing::Assert::Throws<awl::io::IoException>([&v, &current_header]()
    {
        const size_t old_version = 1u;

        const awl::io::Header old_header{ current_header.format, old_version };

        awl::io::VectorInputStream in(v);

        v2::B b;
        Value val(old_header, b);
        val.Read(in);
    });

    // Read a newer version from an older stream.
    {
        const size_t new_version = 3u;

        const awl::io::Header new_header{ current_header.format, new_version };

        awl::io::VectorInputStream in(v);

        v2::B b;
        Value val(new_header, b);
        val.Read(in);

        // Check we fall back to the old version.
        AWT_ASSERT(val.oldVersion == current_header.version);
    }
}
