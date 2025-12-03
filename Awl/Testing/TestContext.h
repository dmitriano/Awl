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

        CompositeTestContext(Logger& logger, const std::stop_token stopToken, Provider& ap) :
            logger(logger), stopToken(stopToken), ap(ap)
        {}

        Logger& logger;

        const std::stop_token stopToken;

        Provider& ap;
    };

#ifdef AWL_QT

    struct TestContext : public CompositeTestContext<CommandLineProvider, JsonProvider>
    {
        using Base = CompositeTestContext<CommandLineProvider, JsonProvider>;

        TestContext(Logger& logger, const std::stop_token stopToken, Provider& ap, QObject* worker = nullptr) :
            Base(logger, stopToken, ap),
            worker(worker)
        {}

        // For handling QT signals inside the tests.
        QObject* worker;
    };

#else

    using TestContext = CompositeTestContext<CommandLineProvider>;

#endif

    static_assert(attribute_provider<TestContext::Provider>);
}
