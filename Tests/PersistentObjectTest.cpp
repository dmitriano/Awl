#include "Awl/Testing/UnitTest.h"

#include "Awl/Io/PersistentObject.h"
#include "Awl/Decimal.h"
#include "Awl/ConsoleLogger.h"

#include <map>

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

        bool dumpMode;

        bool operator == (const Settings&) const = default;

        AWL_REFLECT(displayAsset, crossAsset, soundVolume, lockAllMarkets, traceSignals, botSettings,
            dumpMode)
    };

    template <class String>
    Settings<String> MakeSettings()
    {
        typename Settings<String>::BotSettingsMap map{ { String("BTCUSDT"), BotSettings<String>{ String("GridBot"), true } } };

        return Settings<String>
        {
            "USDT",
            "BTC",
            0.5,
            true,
            false,
            map,
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
    void WriteStorage(const awl::testing::TestContext& context)
    {
        awl::ConsoleLogger logger(context.out);

        Storage<String> storage(logger);

        storage.persistentObject.load(settings_file_name);

        storage.persistentObject.value() = MakeSettings<String>();

        storage.persistentObject.save();
    }

    template <class String>
    void ReadStorage(const awl::testing::TestContext& context)
    {
        awl::ConsoleLogger logger(context.out);

        Storage<String> storage(logger);

        storage.persistentObject.load(settings_file_name);

        AWT_ASSERT(storage.persistentObject.value() == MakeSettings<String>());
    }
}

AWT_TEST(PersistentObject)
{
    WriteStorage<std::string>(context);

#ifdef AWL_QT
    ReadStorage<QString>(context);
#else
    ReadStorage<std::string>(context);
#endif
}
