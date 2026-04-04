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
        using AttributeProvider = CompositeProvider<Ps...>;

        CompositeTestContext(Logger& logger, const std::stop_token stop_token, AttributeProvider& attribute_provider) :
            logger(logger), stopToken(stop_token), attributeProvider(attribute_provider)
        {}

        Logger& logger;

        const std::stop_token stopToken;

        AttributeProvider& attributeProvider;
    };

#ifdef AWL_QT

    struct TestContext : public CompositeTestContext<CommandLineProvider, JsonProvider>
    {
        using Base = CompositeTestContext<CommandLineProvider, JsonProvider>;

        TestContext(Logger& logger, const std::stop_token stop_token, AttributeProvider& attribute_provider, QObject* worker = nullptr) :
            Base(logger, stop_token, attribute_provider),
            worker(worker)
        {}

        // For handling QT signals inside the tests.
        QObject* worker;
    };

#else

    using TestContext = CompositeTestContext<CommandLineProvider>;

#endif

    static_assert(attribute_provider<TestContext::AttributeProvider>);
}
