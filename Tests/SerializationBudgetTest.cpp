#include "Awl/Io/ReadWrite.h"
#include "Awl/Io/HeaderedSerializable.h"
#include "Awl/Io/VectorStream.h"
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
