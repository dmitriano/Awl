#include "Awl/Io/ReadWrite.h"
#include "Awl/Io/HeaderedSerializable.h"
#include "Awl/Io/VectorStream.h"
#include "Awl/Io/Reader.h"
#include "Awl/Io/PlainReader.h"
#include "Awl/Io/Writer.h"
#include "Awl/Testing/UnitTest.h"

#ifdef AWL_QT
#include "QtExtras/Io/Rw/ByteArrayReadWrite.h"
#endif

#include <array>
#include <limits>
#include <string>
#include <vector>

namespace
{
    using namespace awl::io;

    template <class T>
    std::vector<std::byte> serializeBudgetSample(const T& value)
    {
        std::vector<std::byte> bytes;
        VectorOutputStream out(bytes);
        write(out, value);
        return bytes;
    }

    template <class T>
    void checkBudgetRoundTrip(const T& expected, size_t limit)
    {
        const auto bytes = serializeBudgetSample(expected);
        VectorInputStream in(bytes);
        const LimitedContext ctx(limit);
        T actual;
        read(in, actual, ctx);
        AWL_ASSERT(actual == expected);
        AWL_ASSERT(in.end());
        awl::testing::Assert::throws<IoError>([&] { ctx.consume(1); });

        VectorInputStream short_in(bytes);
        T rejected;
        awl::testing::Assert::throws<IoError>([&]
        {
            read(short_in, rejected, LimitedContext(limit - 1));
        });
    }

    // The concept needs consume() only, not a public getter for the limit.
    struct BudgetAdapter
    {
        const LimitedContext& budget;
        void consume(size_t byte_count) const { budget.consume(byte_count); }
    };

    static_assert(limited_context<BudgetAdapter>);
    static_assert(!limited_context<FakeContext>);

    struct BudgetRecord
    {
        std::string name;
        std::vector<int> values;
        AWL_REFLECT(name, values)
    };

    struct OldBudgetRecord
    {
        std::string removed;
        int value;
        AWL_REFLECT(removed, value)
    };

    struct NewBudgetRecord
    {
        int value;
        AWL_REFLECT(value)
    };

    using BudgetVariant = awl::mp::variant_from_struct<BudgetRecord>;
    using DefaultBudgetReader = Reader<BudgetVariant, VectorInputStream>;
    using LimitedBudgetReader = Reader<BudgetVariant, VectorInputStream, LimitedContext>;
    using LimitedPlainBudgetReader = PlainReader<BudgetVariant, VectorInputStream, LimitedContext>;

    static_assert(std::default_initializable<DefaultBudgetReader>);
    static_assert(!std::default_initializable<LimitedBudgetReader>);
    static_assert(std::constructible_from<LimitedBudgetReader, size_t>);
    static_assert(!limited_context<DefaultBudgetReader>);
    static_assert(limited_context<LimitedBudgetReader>);

    struct BudgetCounter
    {
        explicit BudgetCounter(size_t& byte_count) : count(byte_count) {}
        void consume(size_t byte_count) const { count += byte_count; }
        size_t& count;
    };

    template <class T>
    std::vector<std::byte> serializeBudgetRecord(const T& value, bool with_metadata = true)
    {
        std::vector<std::byte> bytes;
        VectorOutputStream out(bytes);
        Writer<awl::mp::variant_from_struct<T>, VectorOutputStream> writer;
        if (with_metadata)
        {
            writer.writeNewPrototypes(out);
        }
        writer.writeV(out, value);
        return bytes;
    }
}

AWL_TEST(SerializationBudgetReader)
{
    AWL_UNUSED_CONTEXT;
    const BudgetRecord expected{"budget", {1, 2, 3}};
    const auto bytes = serializeBudgetRecord(expected);
    size_t consumed = 0;
    Reader<BudgetVariant, VectorInputStream, BudgetCounter> counter(consumed);
    VectorInputStream count_in(bytes);
    counter.readOldPrototypes(count_in);
    const size_t metadata_cost = consumed;
    BudgetRecord counted;
    read(count_in, counted, counter); // Dispatch through the versioned context.
    AWL_ASSERT(counted.name == expected.name && counted.values == expected.values);
    AWL_ASSERT(metadata_cost > 0 && consumed > metadata_cost);

    VectorInputStream in(bytes);
    LimitedBudgetReader reader(consumed);
    reader.readOldPrototypes(in);
    BudgetRecord actual;
    read(in, actual, reader);
    AWL_ASSERT(actual.name == expected.name && actual.values == expected.values);
    AWL_ASSERT(in.end());
    awl::testing::Assert::throws<IoError>([&] { reader.consume(1); });

    VectorInputStream metadata_in(bytes);
    LimitedBudgetReader short_metadata(metadata_cost - 1);
    awl::testing::Assert::throws<IoError>([&] { short_metadata.readOldPrototypes(metadata_in); });

    VectorInputStream short_in(bytes);
    LimitedBudgetReader short_reader(consumed - 1);
    short_reader.readOldPrototypes(short_in);
    awl::testing::Assert::throws<IoError>([&] { short_reader.readV(short_in, actual); });

    VectorInputStream index_in(bytes);
    LimitedBudgetReader index_reader(metadata_cost);
    index_reader.readOldPrototypes(index_in);
    awl::testing::Assert::throws<IoError>([&] { index_reader.readV(index_in, actual); });
    uint16_t first_index;
    read(index_in, first_index); // The rejected index read must not advance the stream.
    AWL_ASSERT(first_index == 0);

    VectorInputStream unlimited_in(bytes);
    DefaultBudgetReader unlimited_reader;
    unlimited_reader.readOldPrototypes(unlimited_in);
    unlimited_reader.readV(unlimited_in, actual);
    AWL_ASSERT(actual.name == expected.name && actual.values == expected.values);
    AWL_ASSERT(unlimited_in.end());
}

AWL_TEST(SerializationBudgetPlainReader)
{
    AWL_UNUSED_CONTEXT;
    const BudgetRecord expected{"plain", {4, 5}};
    const auto bytes = serializeBudgetRecord(expected, false);
    const size_t limit = bytes.size() + expected.name.size() + expected.values.size() * sizeof(int);
    VectorInputStream in(bytes);
    LimitedPlainBudgetReader reader(limit);
    BudgetRecord actual;
    read(in, actual, reader);
    AWL_ASSERT(actual.name == expected.name && actual.values == expected.values);
    AWL_ASSERT(in.end());
    awl::testing::Assert::throws<IoError>([&] { reader.consume(1); });

    VectorInputStream index_in(bytes);
    LimitedPlainBudgetReader index_reader(0);
    awl::testing::Assert::throws<IoError>([&] { index_reader.readV(index_in, actual); });
    uint16_t first_index;
    read(index_in, first_index);
    AWL_ASSERT(first_index == 0);
}

AWL_TEST(SerializationBudgetSkippedField)
{
    AWL_UNUSED_CONTEXT;
    const OldBudgetRecord expected{"discarded", 42};
    const auto bytes = serializeBudgetRecord(expected, false);
    using Versions = awl::mp::variant_from_structs<OldBudgetRecord, NewBudgetRecord>;
    const size_t limit = bytes.size() + expected.removed.size();

    // Use the old writer's metadata, while reading into the new structure.
    const auto metadata = serializeBudgetRecord(expected);
    VectorInputStream in(metadata);
    Reader<Versions, VectorInputStream, LimitedContext> reader(limit);
    Metadata old_metadata;
    read(in, old_metadata);
    reader.attachMetadata(old_metadata);
    NewBudgetRecord actual{};
    reader.readV(in, actual);
    AWL_ASSERT(actual.value == expected.value);
    AWL_ASSERT(in.end());
    awl::testing::Assert::throws<IoError>([&] { reader.consume(1); });

    VectorInputStream short_in(bytes);
    Reader<Versions, VectorInputStream, LimitedContext> short_reader(limit - 1);
    short_reader.attachMetadata(old_metadata);
    awl::testing::Assert::throws<IoError>([&] { short_reader.readV(short_in, actual); });
}

AWL_TEST(SerializationBudgetCumulative)
{
    AWL_UNUSED_CONTEXT;
    const LimitedContext ctx(4);
    ctx.consume(0);
    ctx.consume(1);
    ctx.consume(3);
    ctx.consume(0);
    awl::testing::Assert::throws<IoError>([&] { ctx.consume(1); });

    const LimitedContext large_ctx(std::numeric_limits<size_t>::max());
    large_ctx.consume(std::numeric_limits<size_t>::max() - 1);
    awl::testing::Assert::throws<IoError>([&] { large_ctx.consume(2); });
    large_ctx.consume(1); // A rejected request did not change the counter.
}

AWL_TEST(SerializationBudgetRawRead)
{
    AWL_UNUSED_CONTEXT;
    const std::array<std::byte, 3> expected{std::byte{1}, std::byte{2}, std::byte{3}};
    const auto bytes = serializeBudgetSample(expected);
    VectorInputStream in(bytes);
    const LimitedContext budget(2);
    const BudgetAdapter ctx{budget};
    std::array<std::byte, 3> actual{};
    awl::testing::Assert::throws<IoError>([&] { read(in, actual, ctx); });
    AWL_ASSERT((actual == std::array<std::byte, 3>{}));

    // Rejection happened before advancing the stream.
    read(in, actual);
    AWL_ASSERT(actual == expected);
}

AWL_TEST(SerializationBudgetContainers)
{
    AWL_UNUSED_CONTEXT;
    checkBudgetRoundTrip(std::string("abc"), sizeof(size_t) + 3 + 3);
    checkBudgetRoundTrip(std::wstring(L"abc"), sizeof(size_t) + 2 * 3 * sizeof(wchar_t));
    checkBudgetRoundTrip(std::vector<int>{1, 2, 3}, sizeof(size_t) + 2 * 3 * sizeof(int));
    checkBudgetRoundTrip(std::vector<bool>(8, true), sizeof(size_t) + 2);
    checkBudgetRoundTrip(std::vector<bool>{true, false, true, false, true, false, true, false, true},
        sizeof(size_t) + 2 + 2);
    checkBudgetRoundTrip(std::vector<bool>{}, sizeof(size_t));
    checkBudgetRoundTrip(std::vector<std::string>{"", "abc", "d"},
        sizeof(size_t) + 3 * sizeof(std::string) + 3 * sizeof(size_t) + 4 + 4);
    checkBudgetRoundTrip(std::vector<std::vector<int>>{{1, 2}, {}, {3}},
        sizeof(size_t) + 3 * sizeof(std::vector<int>) + 3 * sizeof(size_t) + 2 * 3 * sizeof(int));
    checkBudgetRoundTrip(std::vector<std::string>{}, sizeof(size_t));
}

AWL_TEST(SerializationBudgetTruncatedRead)
{
    AWL_UNUSED_CONTEXT;
    const std::vector<std::byte> bytes(2);
    VectorInputStream in(bytes);
    const LimitedContext ctx(4);
    std::array<std::byte, 4> buffer;
    awl::testing::Assert::throws<EndOfFileException>([&]
    {
        readRaw(in, buffer.data(), buffer.size(), ctx);
    });
    AWL_ASSERT(in.end());
    awl::testing::Assert::throws<IoError>([&] { ctx.consume(1); });
}

AWL_TEST(SerializationBudgetBeforeResize)
{
    AWL_UNUSED_CONTEXT;
    const auto bytes = serializeBudgetSample(std::numeric_limits<size_t>::max());
    const auto reject_resize = [&](auto& value)
    {
        VectorInputStream in(bytes);
        awl::testing::Assert::throws<IoError>([&] { read(in, value, LimitedContext(128)); });
    };

    std::string text = "unchanged";
    reject_resize(text);
    AWL_ASSERT(text == "unchanged");
    std::vector<std::string> strings{"unchanged"};
    reject_resize(strings);
    AWL_ASSERT(strings == std::vector<std::string>{"unchanged"});
    std::vector<bool> bits{true};
    reject_resize(bits);
    AWL_ASSERT(bits == std::vector<bool>{true});

    std::wstring wide = L"unchanged";
    reject_resize(wide);
    AWL_ASSERT(wide == L"unchanged");
    std::vector<int> numbers{42};
    reject_resize(numbers);
    AWL_ASSERT(numbers == std::vector<int>{42});

    // Element count fits, but the estimated storage does not. Reject before resize.
    const auto small_bytes = serializeBudgetSample(size_t{3});
    VectorInputStream strings_in(small_bytes);
    awl::testing::Assert::throws<IoError>([&]
    {
        read(strings_in, strings, LimitedContext(sizeof(size_t) + 3));
    });
    AWL_ASSERT(strings == std::vector<std::string>{"unchanged"});

    VectorInputStream numbers_in(small_bytes);
    awl::testing::Assert::throws<IoError>([&]
    {
        read(numbers_in, numbers, LimitedContext(sizeof(size_t) + 3));
    });
    AWL_ASSERT(numbers == std::vector<int>{42});
}

AWL_TEST(SerializationBudgetHeader)
{
    AWL_UNUSED_CONTEXT;
    const Header expected{std::string(64, 'x'), 4};
    const size_t header_limit = 2 * sizeof(size_t) + 2 * expected.format.size();
    const auto bytes = serializeBudgetSample(expected);
    VectorInputStream in(bytes);
    Header actual;
    const LimitedContext ctx(header_limit);
    read(in, actual, ctx);
    AWL_ASSERT(actual.format == expected.format && actual.version == expected.version);
    awl::testing::Assert::throws<IoError>([&] { ctx.consume(1); });

    VectorInputStream short_in(bytes);
    awl::testing::Assert::throws<IoError>([&]
    {
        read(short_in, actual, LimitedContext(header_limit - 1));
    });
}

#ifdef AWL_QT
AWL_TEST(SerializationBudgetQtByteArray)
{
    AWL_UNUSED_CONTEXT;
    checkBudgetRoundTrip(QByteArray("abc"), sizeof(size_t) + 3 + 3);

    const auto bytes = serializeBudgetSample(std::numeric_limits<size_t>::max());
    VectorInputStream in(bytes);
    QByteArray value("unchanged");
    awl::testing::Assert::throws<IoError>([&] { read(in, value, LimitedContext(128)); });
    AWL_ASSERT(value == "unchanged");
}
#endif
