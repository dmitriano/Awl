/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Tests/Experimental/ObservableFunction.h"
#include "Awl/Testing/UnitTest.h"

#include <functional>
#include <utility>

namespace
{
    using FunctionHandler = awl::Observer<std::function<void(int)>>;

    class FunctionObservable : public awl::Observable<std::function<void(int)>>
    {
    public:

        void emit(int val)
        {
            notify(val);
        }
    };

    using PredicateHandler = awl::Observer<std::function<bool(int)>>;

    class PredicateObservable : public awl::Observable<std::function<bool(int)>>
    {
    public:

        bool emitWhileTrue(int val)
        {
            return notifyWhileTrue(val);
        }
    };

    class FunctionAndPredicateOwner
    {
    public:

        FunctionAndPredicateOwner()
            : functionHandler(std::bind(&FunctionAndPredicateOwner::onFunction, this, std::placeholders::_1))
            , predicateHandler(std::bind(&FunctionAndPredicateOwner::onPredicate, this, std::placeholders::_1))
        {
        }

        void onFunction(int value)
        {
            ++functionCallCount;
            lastFunctionValue = value;
        }

        bool onPredicate(int value)
        {
            ++predicateCallCount;
            lastPredicateValue = value;
            return predicateResult;
        }

        FunctionHandler functionHandler;
        PredicateHandler predicateHandler;

        int functionCallCount = 0;
        int predicateCallCount = 0;
        int lastFunctionValue = 0;
        int lastPredicateValue = 0;
        bool predicateResult = true;
    };
}

AWL_TEST(Observable_Function_Events)
{
    AWL_UNUSED_CONTEXT;

    FunctionObservable observable;

    FunctionHandler handler1;
    FunctionHandler handler2;
    FunctionHandler handler3;

    int callCount1 = 0;
    int callCount2 = 0;
    int callCount3 = 0;

    handler1.setFunction([&](int) { ++callCount1; });
    handler2.setFunction([&](int) { ++callCount2; });
    handler3.setFunction([&](int) { ++callCount3; });

    observable.subscribe(&handler1);
    observable.subscribe(&handler2);
    observable.subscribe(&handler3);

    AWL_ASSERT_EQUAL(3u, observable.size());

    observable.emit(1);

    AWL_ASSERT_EQUAL(1, callCount1);
    AWL_ASSERT_EQUAL(1, callCount2);
    AWL_ASSERT_EQUAL(1, callCount3);

    handler1.unsubscribeSelf();
    observable.unsubscribe(&handler2);

    observable.emit(2);

    AWL_ASSERT_EQUAL(1, callCount1);
    AWL_ASSERT_EQUAL(1, callCount2);
    AWL_ASSERT_EQUAL(2, callCount3);
}

AWL_TEST(Observable_Function_NotifyWhileTrue)
{
    AWL_UNUSED_CONTEXT;

    PredicateObservable observable;

    PredicateHandler handler1;
    PredicateHandler handler2;
    PredicateHandler handler3;

    int callCount1 = 0;
    int callCount2 = 0;
    int callCount3 = 0;

    handler1.setFunction([&](int) { ++callCount1; return true; });
    handler2.setFunction([&](int) { ++callCount2; return false; });
    handler3.setFunction([&](int) { ++callCount3; return true; });

    observable.subscribe(&handler1);
    observable.subscribe(&handler2);
    observable.subscribe(&handler3);

    const bool result = observable.emitWhileTrue(10);

    AWL_ASSERTM_FALSE(result, _T("notifyWhileTrue should stop at false."));
    AWL_ASSERT_EQUAL(1, callCount1);
    AWL_ASSERT_EQUAL(1, callCount2);
    AWL_ASSERT_EQUAL(0, callCount3);
}

AWL_TEST(Observable_FunctionObserver_Move)
{
    AWL_UNUSED_CONTEXT;

    FunctionObservable observable;

    FunctionHandler handler1;
    FunctionHandler handler2;

    int callCount1 = 0;
    int callCount2 = 0;

    handler1.setFunction([&](int) { ++callCount1; });
    handler2.setFunction([&](int) { ++callCount2; });

    observable.subscribe(&handler1);
    observable.subscribe(&handler2);

    FunctionHandler handler1_copy = std::move(handler1);

    FunctionHandler handler2_copy;
    handler2_copy = std::move(handler2);

    observable.emit(5);

    AWL_ASSERTM_FALSE(handler1.isSubscribed(), _T("Observer #1 is included"));
    AWL_ASSERTM_FALSE(handler2.isSubscribed(), _T("Observer #2 is included"));

    AWL_ASSERTM_TRUE(handler1_copy.isSubscribed(), _T("Observer copy #1 is not included"));
    AWL_ASSERTM_TRUE(handler2_copy.isSubscribed(), _T("Observer copy #2 is not included"));

    AWL_ASSERT_EQUAL(1, callCount1);
    AWL_ASSERT_EQUAL(1, callCount2);
}

AWL_TEST(Observable_FunctionHandlers_MemberFunctions)
{
    AWL_UNUSED_CONTEXT;

    FunctionObservable functionObservable;
    PredicateObservable predicateObservable;

    FunctionAndPredicateOwner owner;

    functionObservable.subscribe(&owner.functionHandler);
    predicateObservable.subscribe(&owner.predicateHandler);

    functionObservable.emit(11);
    const bool result1 = predicateObservable.emitWhileTrue(13);

    AWL_ASSERT_EQUAL(1, owner.functionCallCount);
    AWL_ASSERT_EQUAL(11, owner.lastFunctionValue);
    AWL_ASSERT_EQUAL(1, owner.predicateCallCount);
    AWL_ASSERT_EQUAL(13, owner.lastPredicateValue);
    AWL_ASSERTM_TRUE(result1, _T("Predicate member-function handler should return true."));

    owner.predicateResult = false;

    const bool result2 = predicateObservable.emitWhileTrue(17);

    AWL_ASSERT_EQUAL(2, owner.predicateCallCount);
    AWL_ASSERT_EQUAL(17, owner.lastPredicateValue);
    AWL_ASSERTM_FALSE(result2, _T("Predicate member-function handler should return false."));
}
