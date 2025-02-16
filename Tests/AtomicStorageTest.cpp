/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Helpers/BenchmarkHelpers.h"

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

#include <memory>
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

        AWL_ASSERT(len != 0);

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
    using HashInputStream = awl::io::HashInputStream<Hash>;
    using HashOutputStream = awl::io::HashOutputStream<Hash>;
    using HashingSerializable = awl::io::HashingSerializable<>;

    using Value1 = awl::io::VersionTolerantSerializable<v1::B, HashInputStream, HashOutputStream, true, V1>;
    using Value2 = awl::io::VersionTolerantSerializable<v2::B, HashInputStream, HashOutputStream, true, V2>;

    template <class Value>
    bool LoadValue(awl::io::AtomicStorage& storage, Value& val, bool existed = true)
    {
        AWL_ASSERT(storage.Open(master_name, backup_name) == existed);
        return storage.Load(val);
    }
}

AWL_TEST(AtomicStoragePlain)
{
    auto guard = awl::make_scope_guard(RemoveFiles);

    auto& logger = context.logger;

    using Value = awl::io::PlainSerializable<v2::B, HashInputStream, HashOutputStream>;

    {
        v2::B b = v2::b_expected;
        Value val(b);
        HashingSerializable hashed_val(val);
        awl::io::AtomicStorage storage(logger);
        AWL_ASSERT(!LoadValue(storage, hashed_val, false));
        AWL_ASSERT(b == v2::b_expected);
        storage.Save(hashed_val);
    }

    {
        v2::B b;
        Value val(b);
        HashingSerializable hashed_val(val);
        awl::io::AtomicStorage storage(logger);
        AWL_ASSERT(LoadValue(storage, hashed_val));
        AWL_ASSERT(b == v2::b_expected);
    }

    CorruptFile(master_name);

    {
        v2::B b = {};
        const v2::B saved_b = b;
        Value val(b);
        HashingSerializable hashed_val(val);
        awl::io::AtomicStorage storage(logger);
        AWL_ASSERT(!LoadValue(storage, hashed_val));
        AWL_ASSERT(b == saved_b);

        b = v2::b_expected;
        storage.Save(hashed_val);
    }

    auto load = [&logger]()
    {
        v2::B b;
        Value val(b);
        HashingSerializable hashed_val(val);
        awl::io::AtomicStorage storage(logger);
        AWL_ASSERT(LoadValue(storage, hashed_val));
        AWL_ASSERT(b == v2::b_expected);
    };

    load();

    SwapFiles();

    load();

    DuplicateMaserFile();
    CorruptFile(master_name);

    load();
}

AWL_TEST(AtomicStorageVts)
{
    auto guard = awl::make_scope_guard(RemoveFiles);

    auto& logger = context.logger;

    {
        {
            v1::B b = v1::b_expected;
            Value1 val(b);
            HashingSerializable hashed_val(val);
            awl::io::AtomicStorage storage(logger);
            AWL_ASSERT(!storage.IsOpened());
            AWL_ASSERT(!LoadValue(storage, hashed_val, false));
            AWL_ASSERT(storage.IsOpened());
            AWL_ASSERT(b == v1::b_expected);
            storage.Save(hashed_val);
        }

        {
            v1::B b;
            Value1 val(b);
            HashingSerializable hashed_val(val);
            awl::io::AtomicStorage storage(logger);
            AWL_ASSERT(LoadValue(storage, hashed_val));
            AWL_ASSERT(b == v1::b_expected);
        }
    }

    {
        v2::B b;
        Value2 val(b);
        HashingSerializable hashed_val(val);
        awl::io::AtomicStorage storage(logger);
        AWL_ASSERT(LoadValue(storage, hashed_val));
        AWL_ASSERT(b == v2::b_expected);
    }

    CorruptFile(master_name);

    {
        v2::B b = {};
        const v2::B saved_b = b;
        Value2 val(b);
        HashingSerializable hashed_val(val);
        awl::io::AtomicStorage storage(logger);
        AWL_ASSERT(!LoadValue(storage, hashed_val));
        AWL_ASSERT(b == saved_b);

        b = v2::b_expected;
        storage.Save(hashed_val);
    }

    auto load = [&logger]()
    {
        v2::B b;
        Value2 val(b);
        HashingSerializable hashed_val(val);
        awl::io::AtomicStorage storage(logger);
        AWL_ASSERT(LoadValue(storage, hashed_val));
        AWL_ASSERT(b == v2::b_expected);
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

AWL_TEST(AtomicStorageMove)
{
    auto guard = awl::make_scope_guard(RemoveFiles);

    auto& logger = context.logger;

    awl::io::AtomicStorage storage = MakeStorage(logger);

    {
        v1::B b = v1::b_expected;
        Value1 val(b);
        HashingSerializable hashed_val(val);
        AWL_ASSERT(!storage.Load(hashed_val));
        AWL_ASSERT(b == v1::b_expected);
        storage.Save(hashed_val);
    }

    awl::io::AtomicStorage storage1 = std::move(storage);

    {
        v2::B b;
        Value2 val(b);
        HashingSerializable hashed_val(val);
        storage1.Load(hashed_val);
        AWL_ASSERT(b == v2::b_expected);
    }
}

AWL_TEST(AtomicStorageSave)
{
    auto guard = awl::make_scope_guard(RemoveFiles);

    auto& logger = context.logger;

    awl::io::AtomicStorage storage = MakeStorage(logger);
    AWL_ASSERT(storage.IsEmpty());

    {
        v1::B b = v1::b_expected;
        Value1 val(b);
        HashingSerializable hashed_val(val);
        storage.Save(hashed_val);
        AWL_ASSERT(!storage.IsEmpty());
    }

    awl::io::AtomicStorage storage1 = std::move(storage);
    AWL_ASSERT(!storage1.IsEmpty());

    {
        v2::B b;
        Value2 val(b);
        HashingSerializable hashed_val(val);
        storage1.Load(hashed_val);
        AWL_ASSERT(b == v2::b_expected);
    }
}

namespace
{
    void EuphoricalLoadSaveTest(const awl::testing::TestContext& context, awl::IMutex* p_mutex)
    {
        auto guard = awl::make_scope_guard(RemoveFiles);

        auto& logger = context.logger;

        awl::io::AtomicStorage storage = MakeStorage(logger);
        AWL_ASSERT(storage.IsEmpty());

        using Value = awl::io::EuphoricallySerializable<v2::B>; //, V2

        {
            v2::B b = v2::b_expected;
            Value val(b);
            storage.Save(val, p_mutex);
        }

        {
            v2::B b;
            Value val(b);
            storage.Load(val);
            AWL_ASSERT(b == v2::b_expected);
        }
    }
}

AWL_TEST(AtomicStorageEuphorical1)
{
    EuphoricalLoadSaveTest(context, nullptr);

    std::mutex mutex;
    awl::MutexWrapper wrapper(mutex);

    EuphoricalLoadSaveTest(context, &wrapper);
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

        auto& logger = context.logger;

        awl::io::AtomicStorage storage = MakeStorage(logger);
        AWL_ASSERT(storage.IsEmpty());

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
            AWL_ASSERT(gp2.clusterType == ClusterType::Line);
            AWL_ASSERT(gp2.as_v1_tuple() == gp1_sample.as_tuple());
        }
    }
}

AWL_TEST(AtomicStorageVts2)
{
    TestRenameAndDefaultFields<
        awl::io::VersionTolerantSerializable<GameParamsV1>, //, awl::mp::variant_from_structs<GameParamsV1>
        awl::io::VersionTolerantSerializable<GameParamsV2>> //, awl::mp::variant_from_structs<GameParamsV2>
    (context);
}

AWL_TEST(AtomicStorageEuphorical2)
{
    TestRenameAndDefaultFields<
        awl::io::EuphoricallySerializable<GameParamsV1>, //, awl::mp::variant_from_structs<GameParamsV1>
        awl::io::EuphoricallySerializable<GameParamsV2>> //, awl::mp::variant_from_structs<GameParamsV2>
    (context);
}

AWL_TEST(Shapshot)
{
    using Value = awl::io::EuphoricallySerializable<v2::B, awl::io::VectorInputStream, awl::io::VectorOutputStream>; //, V2

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

    AWL_ASSERT(actual_v == expected_v);

    context.out << _T("Snapshot size: ") << snapshot.size() << _T(" bytes") << std::endl;
    context.out << _T("Snapshot size: ") << actual_v.size() << _T(" bytes") << std::endl;
    context.out << _T("Hash size: ") << actual_v.size() - snapshot.size() << _T(" bytes") << std::endl;
}

namespace
{
    class HeaderedValue : public awl::io::HeaderedSerializable<v2::B, awl::io::VectorInputStream, awl::io::VectorOutputStream> //, V2
    {
    private:

        using Base = awl::io::HeaderedSerializable<v2::B, awl::io::VectorInputStream, awl::io::VectorOutputStream>;//, V2

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

AWL_TEST(HeaderedSerializable)
{
    AWL_UNUSED_CONTEXT;

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
        AWL_ASSERT(b == v2::b_expected);
        // Check the versions match.
        AWL_ASSERT(val.oldVersion == HeaderedValue::NoVersion);
    }

    awl::testing::Assert::Throws<awl::io::IoException>([&v, &current_header]()
    {
        const awl::io::Header wrong_header{ "WRONG FORMAT", current_header.version };

        awl::io::VectorInputStream in(v);

        v2::B b;
        Value val(wrong_header, b);
        val.Read(in);
        AWL_ASSERT(b == v2::b_expected);
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
        AWL_ASSERT(val.oldVersion == current_header.version);
    }

    // Exceed format name limit.

    awl::testing::Assert::Throws<awl::io::IoException>([&v, &current_header]()
    {
        awl::io::VectorInputStream in(v);

        v2::B b;
        Value val(current_header, b, awl::io::defaultBlockSize, {}, 5u);
        val.Read(in);
    });

    v.clear();

    awl::testing::Assert::Throws<awl::io::IoException>([&v, &current_header]()
    {
        awl::io::VectorOutputStream out(v);

        v2::B b = v2::b_expected;
        Value val(current_header, b, awl::io::defaultBlockSize, {}, 5u);
        val.Write(out);
    });
}

namespace
{
    struct SomeState
    {
        size_t count;

        std::vector<v2::B> b;

        AWL_REFLECT(count, b)
    };
}

AWL_BENCHMARK(AtomicStorageVtsWrite)
{
    using signed_size_t = std::make_signed_t<size_t>;

    AWL_ATTRIBUTE(size_t, write_count, 100u);
    AWL_FLAG(fake_hash); // To ensure that hash does not take a significant time.
    AWL_FLAG(vector_stream); // For comparing with writing to a file.
    AWL_ATTRIBUTE(size_t, element_count, 1);
    AWL_ATTRIBUTE(signed_size_t, z_size, -1);
    AWL_ATTRIBUTE(signed_size_t, v_size, -1);
    AWL_ATTRIBUTE(signed_size_t, v2_size, -1);

    auto guard = awl::make_scope_guard(RemoveFiles);

    auto& logger = context.logger;

    SomeState state{ 0u, {} };

    v2::B b = v2::b_expected;

    if (z_size != -1)
    {
        b.z.resize(static_cast<size_t>(z_size));
    }

    if (v_size != -1)
    {
        b.z.resize(static_cast<size_t>(v_size));
    }

    if (v2_size != -1)
    {
        b.z.resize(static_cast<size_t>(v2_size));
    }

    state.b.resize(element_count, b);

    awl::io::Header header{ "SAMPLE FORMAT", 1u };

    std::unique_ptr<awl::io::Serializable<>> p_val;

    if (fake_hash)
    {
        p_val = std::make_unique<awl::io::HeaderedSerializable<SomeState, awl::io::SequentialInputStream, awl::io::SequentialOutputStream,
            awl::crypto::FakeHash>>(std::move(header), state);
    }
    else
    {
        p_val = std::make_unique<awl::io::HeaderedSerializable<SomeState>>(std::move(header), state);
    }

    size_t stream_size;

    {
        awl::io::MeasureStream measure_out;

        p_val->Write(measure_out);

        stream_size = measure_out.GetLength();

        context.out << "Stream Size: " << stream_size << " bytes." << std::endl;
    }

    if (vector_stream)
    {
        std::vector<uint8_t> v;
        v.resize(stream_size);

        awl::io::VectorOutputStream out(v);

        awl::StopWatch w;

        for (size_t i : awl::make_count(write_count))
        {
            state.count = i;

            awl::io::WriteV(out, state);
        }

        helpers::ReportCount(context, w, write_count);
    }
    else
    {
        {
            awl::io::AtomicStorage storage(logger);
            storage.Open(master_name, backup_name);

            storage.Load(*p_val);

            awl::StopWatch w;

            for (size_t i : awl::make_count(write_count))
            {
                state.count = i;

                storage.Save(*p_val);
            }

            helpers::ReportCount(context, w, write_count);

            context.out << std::endl;

            storage.Load(*p_val);

            AWL_ASSERT(state.count == write_count - 1);
        }

        // File Size should be equal to Stream Size printed above.
        context.out << "File Size: " << fs::file_size(master_name) << " bytes." << std::endl;
    }
}
