/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/CompositeCompare.h"
#include "Awl/TransparentCompositeCompare.h"
#include "Awl/KeyCompare.h"
#include "Awl/EnumTraits.h"

#include "Awl/Testing/UnitTest.h"

#include <string>

namespace
{
    struct X
    {
        int a;
        std::string b;
    };

    inline auto MakeKey(const X& x)
    {
        return std::make_tuple(x.a, x.b);
    }

    using ACompare = awl::FieldCompare<X, int, &X::a>;
    using BCompare = awl::FieldCompare<X, std::string, &X::b>;

    static_assert(std::is_same_v<ACompare::key_type, const int&>);
    static_assert(std::is_same_v<BCompare::key_type, const std::string&>);
    static_assert(std::is_same_v<decltype(MakeKey({})), std::tuple<int, std::string>>);

    X x1a{ 1, "a" };
    X x1b{ 1, "b" };
    X x2a{ 2, "a" };
    X x3a{ 3, "a" };
}

using namespace awl::testing;

AWT_TEST(CompositeCompare)
{
    AWT_UNUSED_CONTEXT;

    //auto comp = awl::CompositeCompare<X, ACompare, BCompare>(ACompare(), BCompare());
    auto comp = awl::compose_comparers<X>(ACompare(), BCompare());

    AWT_ASSERT(!comp(x1a, x1a));
    AWT_ASSERT(!comp(x1b, x1b));
    AWT_ASSERT(!comp(x2a, x2a));
    AWT_ASSERT(!comp(x3a, x3a));

    AWT_ASSERT(comp(x1a, x1b));
    AWT_ASSERT(!comp(x1b, x1a));

    AWT_ASSERT(comp(x2a, x3a));
    AWT_ASSERT(!comp(x3a, x2a));
}

AWT_TEST(TransparentCompositeCompareTrivialStructure)
{
    AWT_UNUSED_CONTEXT;

    auto comp = awl::compose_transparent_comparers<X>(ACompare(), BCompare());

    AWT_ASSERT(!comp(x1a, x1a));
    AWT_ASSERT(!comp(x1b, x1b));
    AWT_ASSERT(!comp(x2a, x2a));
    AWT_ASSERT(!comp(x3a, x3a));

    AWT_ASSERT(comp(x1a, x1b));
    AWT_ASSERT(!comp(x1b, x1a));

    AWT_ASSERT(comp(x2a, x3a));
    AWT_ASSERT(!comp(x3a, x2a));

    AWT_ASSERT(!comp(x1a, MakeKey(x1a)));
    AWT_ASSERT(!comp(x1b, MakeKey(x1b)));
    AWT_ASSERT(!comp(x2a, MakeKey(x2a)));
    AWT_ASSERT(!comp(x3a, MakeKey(x3a)));

    AWT_ASSERT(comp(x1a, MakeKey(x1b)));
    AWT_ASSERT(!comp(x1b, MakeKey(x1a)));

    AWT_ASSERT(comp(x2a, MakeKey(x3a)));
    AWT_ASSERT(!comp(x3a, MakeKey(x2a)));

    AWT_ASSERT(!comp(MakeKey(x1a), x1a));
    AWT_ASSERT(!comp(MakeKey(x1b), x1b));
    AWT_ASSERT(!comp(MakeKey(x2a), x2a));
    AWT_ASSERT(!comp(MakeKey(x3a), x3a));

    AWT_ASSERT(comp(MakeKey(x1a), x1b));
    AWT_ASSERT(!comp(MakeKey(x1b), x1a));

    AWT_ASSERT(comp(MakeKey(x2a), x3a));
    AWT_ASSERT(!comp(MakeKey(x3a), x2a));
}

namespace data
{
    AWL_SEQUENTIAL_ENUM(AccountType, Spot, CrossMargin, IsolatedMargin)

    struct Wallet
    {
        AccountType accountType;
        std::string asset;
        double freeBalance;
        double lockedBalance;
        int updateTime;

        AWL_STRINGIZABLE(accountType, asset, freeBalance, lockedBalance, updateTime);
    };

    AWL_MEMBERWISE_EQUATABLE(Wallet)
}

AWL_ENUM_TRAITS(data, AccountType)

namespace
{
    using WalletAccountCompare = awl::FieldCompare<data::Wallet, data::AccountType, &data::Wallet::accountType>;
    using WalletAssetCompare = awl::FieldCompare<data::Wallet, std::string, &data::Wallet::asset>;
    using WalletPrimaryCompare = awl::TransparentCompositeCompare<data::Wallet, WalletAccountCompare, WalletAssetCompare>;
}

AWT_TEST(TransparentCompositeCompareInheritedKey)
{
    AWT_UNUSED_CONTEXT;

    using InheritedWalletKey = WalletPrimaryCompare::key_type;

    static_assert(std::is_same_v<InheritedWalletKey, std::tuple<const data::AccountType&, const std::string&>>);

    const std::string asset = "BTC";

    const double free = 5;
    const double locked = 7;

    const data::AccountType spot_account = data::AccountType::Spot;

    const data::AccountType im_account = data::AccountType::IsolatedMargin;

    const InheritedWalletKey spot_key(spot_account /*data::AccountType::Spot*/, asset);

    const InheritedWalletKey im_key(im_account /*data::AccountType::IsolatedMargin*/, asset);

    const data::Wallet spot_wallet = { data::AccountType::Spot, asset, free, locked, 5 };

    const data::Wallet im_wallet = { data::AccountType::IsolatedMargin, asset, free, locked, 10 };

    WalletPrimaryCompare comp;

    AWT_ASSERT(!comp(spot_key, spot_wallet));
    AWT_ASSERT(comp(spot_key, im_wallet));
    AWT_ASSERT(!comp(im_key, im_wallet));
    AWT_ASSERT(!comp(im_key, spot_wallet));

    // Ensure that universal key converts to inherited key.

    const auto universal_key = WalletPrimaryCompare::make_key(data::AccountType::Spot, "BTC");
    using UniversalKey = decltype(universal_key);
    static_assert(std::is_same_v<UniversalKey, const std::tuple<data::AccountType, std::string>>);

    const InheritedWalletKey inherited_key = universal_key;
    AWT_ASSERT(inherited_key == universal_key);
}

AWT_TEST(TransparentCompositeCompareUniversalKey)
{
    AWT_UNUSED_CONTEXT;

    const std::string asset = "BTC";

    const double free = 5;
    const double locked = 7;

    const data::Wallet spot_wallet = { data::AccountType::Spot, asset, free, locked, 5 };
    const data::Wallet im_wallet = { data::AccountType::IsolatedMargin, asset, free, locked, 10 };

    WalletPrimaryCompare comp;

    AWT_ASSERT(!comp(WalletPrimaryCompare::make_key(data::AccountType::Spot, "BTC"), spot_wallet));
    AWT_ASSERT(comp(WalletPrimaryCompare::make_key(data::AccountType::Spot, asset), im_wallet));
    AWT_ASSERT(!comp(WalletPrimaryCompare::make_key(data::AccountType::IsolatedMargin, asset), im_wallet));
    AWT_ASSERT(!comp(WalletPrimaryCompare::make_key(data::AccountType::IsolatedMargin, asset), spot_wallet));
    AWT_ASSERT(!comp(WalletPrimaryCompare::make_key(data::AccountType::IsolatedMargin, std::string("ETH")), spot_wallet));
    AWT_ASSERT(comp(spot_wallet, WalletPrimaryCompare::make_key(data::AccountType::IsolatedMargin, std::string("ETH"))));
}
