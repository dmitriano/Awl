#include "DataModel.h"

#include "Qtil/VersionableAtomicObject.h"

#include "Qtil/Testing/UnitTest.h"

#include "Awl/Io/PersistentObject.h"
#include "Awl/Io/VectorStream.h"
#include "Awl/Decimal.h"

#include <map>

using namespace tradeclient;

namespace
{
    template <class String>
    struct BotSettings
    {
        String type;
        bool started;

        bool operator == (const BotSettings&) const = default;

        AWL_REFLECT(type, started);
    };

    template <class String>
    struct Settings
    {
        String displayAsset;
        String crossAsset;

        double soundVolume;

        bool lockAllMarkets;

        bool traceSignals;

        using BotSettingsMap = std::unordered_map<String, BotSettings<String>>;
        BotSettingsMap botSettings;

        data::decimal volatilityFactor;

        bool dumpMode;

        bool operator == (const Settings&) const = default;

        AWL_REFLECT(displayAsset, crossAsset, soundVolume, lockAllMarkets, traceSignals, botSettings, volatilityFactor,
            dumpMode)
    };

    template <class String>
    Settings<String> MakeSettings()
    {
        using namespace qtil::literals;

        typename Settings<String>::BotSettingsMap map{ { String("BTCUSDT"), BotSettings<String>{ String("GridBot"), true } } };

        return Settings<String>
        {
            "USDT",
            "BTC",
            0.5,
            true,
            false,
            map,
            "2"_d,
            false
        };
    }

    template <class String>
    struct Storage
    {
        Storage(awl::Logger& logger) :
            persistentObject(logger, awl::io::Header{ "Test Settings", 1u })
        {}

        using V = awl::io::helpers::variant_from_structs<Settings<String>, BotSettings<String>>;

        awl::io::PersistentObject<Settings<String>, V> persistentObject;
    };

    const awl::String settings_file_name = awl::text("test-settings");

    template <class String>
    void WriteStorage(const qtil::testing::TestContext& context)
    {
        Storage<String> storage(context.logger);

        storage.persistentObject.load(settings_file_name);

        storage.persistentObject.value() = MakeSettings<String>();

        storage.persistentObject.save();
    }

    template <class String>
    void ReadStorage(const qtil::testing::TestContext& context)
    {
        Storage<String> storage(context.logger);

        storage.persistentObject.load(settings_file_name);

        AWT_ASSERT(storage.persistentObject.value() == MakeSettings<String>());
    }
}

QTIL_UNIT_TEST(Settings)
{
    WriteStorage<std::string>(context);

    ReadStorage<QString>(context);
}
