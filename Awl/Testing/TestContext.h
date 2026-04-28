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
#include "Awl/Testing/TypeProvider.h"

#ifdef AWL_QT
    #include "Awl/Testing/JsonProvider.h"
    #include <QObject>
#endif

#include "Awl/Testing/CompositeProvider.h"

#include <cassert>
#include <memory>

namespace awl::testing
{
    template <class ... Ps>
    struct CompositeTestContext
    {
        using AttributeProvider = CompositeProvider<Ps...>;

        CompositeTestContext(std::shared_ptr<Logger> logger, const std::stop_token stop_token,
            AttributeProvider& attribute_provider, const TypeProvider& type_provider) :
            logger(std::move(logger)), stopToken(stop_token), attributeProvider(attribute_provider), typeProvider(type_provider)
        {
            assert(this->logger != nullptr);
        }

        std::shared_ptr<Logger> logger;

        const std::stop_token stopToken;

        AttributeProvider& attributeProvider;

        const TypeProvider& typeProvider;
    };

#ifdef AWL_QT

    struct TestContext : public CompositeTestContext<CommandLineProvider, JsonProvider>
    {
        using Base = CompositeTestContext<CommandLineProvider, JsonProvider>;

        TestContext(std::shared_ptr<Logger> logger, const std::stop_token stop_token,
            AttributeProvider& attribute_provider, const TypeProvider& type_provider, QObject* worker = nullptr) :
            Base(std::move(logger), stop_token, attribute_provider, type_provider),
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
