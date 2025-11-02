/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/String.h"
#include "Awl/Sleep.h"
#include "Awl/Logger.h"
#include "Awl/StringFormat.h"
#include "Awl/Testing/CommandLineProvider.h"

#ifdef AWL_QT
    #include "Awl/Testing/JsonProvider.h"
    #include <QObject>
#endif

#include "Awl/Testing/CompositeProvider.h"

namespace awl::testing
{
    template <class ... Ps>
    struct CompositeTestContext
    {
        using Provider = CompositeProvider<Ps...>;

        CompositeTestContext(awl::ostream& out, Logger& logger, const std::stop_token stopToken, Provider& ap) :
            out(out), logger(logger), stopToken(stopToken), ap(ap)
        {}

        // TODO: out was replaced with logger. Remove out.
        // A mutex can be used for synchronizing output operations in multithreaded tests, for example.
        awl::ostream& out;

        Logger& logger;

        const std::stop_token stopToken;

        Provider& ap;
    };

#ifdef AWL_QT

    struct TestContext : public CompositeTestContext<CommandLineProvider, JsonProvider>
    {
        using Base = CompositeTestContext<CommandLineProvider, JsonProvider>;

        TestContext(awl::ostream& out, Logger& logger, const std::stop_token stopToken, Provider& ap) :
            Base(out, logger, stopToken, ap),
            worker(nullptr)
        {}

        // For handling QT signals inside the tests.
        QObject* worker;
    };

#else

    using TestContext = CompositeTestContext<CommandLineProvider>;

#endif

    static_assert(attribute_provider<TestContext::Provider>);
}
