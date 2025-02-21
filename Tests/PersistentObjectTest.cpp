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
            std::move(map),
            false
        };
    }

    template <class Storage, class String>
    struct Container
    {
        Container(awl::Logger& logger) :
            persistentObject(logger, awl::io::Header{ "Test Settings", 1u })
        {}

        awl::io::PersistentObject<Settings<String>, Storage> persistentObject;
    };

    const awl::String settings_file_name = awl::text("test-settings");

    template <class Storage, class String>
    void WriteStorage(const awl::testing::TestContext& context)
    {
        awl::ConsoleLogger logger(context.out);

        Container<Storage, String> container(logger);

        container.persistentObject.open(settings_file_name);

        container.persistentObject.load();

        container.persistentObject.value() = MakeSettings<String>();

        container.persistentObject.save();
    }

    template <class Storage, class String>
    void ReadStorage(const awl::testing::TestContext& context)
    {
        awl::ConsoleLogger logger(context.out);

        Container<Storage, String> container(logger);

        container.persistentObject.open(settings_file_name);

        container.persistentObject.load();

        AWL_ASSERT(container.persistentObject.value() == MakeSettings<String>());
    }
}

AWL_TEST(PersistentObject)
{
    WriteStorage<awl::io::AtomicStorage, std::string>(context);

    ReadStorage<awl::io::AtomicStorage, std::string>(context);

    WriteStorage<awl::io::OptionalStorage, std::string>(context);

    ReadStorage<awl::io::OptionalStorage, std::string>(context);
}
