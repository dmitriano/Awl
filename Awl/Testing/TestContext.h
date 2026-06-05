/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/String.h"
#include "Awl/Sleep.h"
#include "Awl/ILogger.h"
#include "Awl/StringFormat.h"
#include "Awl/Testing/CommandLineProvider.h"
#include "Awl/Testing/TypeProvider.h"

#ifdef AWL_BOOST
    #include "Awl/Testing/JsonProvider.h"
#endif

#ifdef AWL_QT
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

        CompositeTestContext(std::shared_ptr<ILogger> logger, const std::stop_token stop_token,
            AttributeProvider& attribute_provider, const TypeProvider& type_provider) :
            logger(std::move(logger)), stopToken(stop_token), attributeProvider(attribute_provider), typeProvider(type_provider)
        {
            assert(this->logger != nullptr);
        }

        std::shared_ptr<ILogger> logger;

        const std::stop_token stopToken;

        AttributeProvider& attributeProvider;

        const TypeProvider& typeProvider;
    };

#ifdef AWL_BOOST

    using TestContextBase = CompositeTestContext<CommandLineProvider, JsonProvider>;

#else

    using TestContextBase = CompositeTestContext<CommandLineProvider>;

#endif

    struct TestContext : public TestContextBase
    {
        using Base = TestContextBase;

        TestContext(std::shared_ptr<ILogger> logger, const std::stop_token stop_token,
            AttributeProvider& attribute_provider, const TypeProvider& type_provider
#ifdef AWL_QT
            , QObject* worker = nullptr
#endif
            ) :
            Base(std::move(logger), stop_token, attribute_provider, type_provider)
#ifdef AWL_QT
            ,
            worker(worker)
#endif
        {}

#ifdef AWL_QT
        // For handling QT signals inside the tests.
        QObject* worker;
#endif
    };

    static_assert(attribute_provider<TestContext::AttributeProvider>);
}
