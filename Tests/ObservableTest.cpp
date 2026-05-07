/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/String.h"
#include "Awl/StringFormat.h"
#include "Awl/StringFormat.h"
#include "Awl/Observable.h"
#include "Awl/Testing/UnitTest.h"
#include "Awl/Testing/Formatter.h"
#include "Awl/Testing/TimeQueue.h"

#include <vector>

using namespace awl::testing;

namespace
{
    struct INotifySomethingChanged
    {
        virtual void ItChanged(int param, awl::String val) = 0;
    };

    class ChangeHandler : public awl::Observer<INotifySomethingChanged>
    {
    public:

        ChangeHandler() = default;
        
        ChangeHandler(const TestContext & c) : pContext(&c)
        {
        }

        void ItChanged(int param, awl::String val) override;

        bool changeHandled = false;

        const TestContext * pContext = nullptr;
    };

    static_assert(!std::is_copy_assignable_v<ChangeHandler>);
    static_assert(!std::is_copy_constructible_v<ChangeHandler>);
    static_assert(std::is_move_assignable_v<ChangeHandler>);
    static_assert(std::is_move_constructible_v<ChangeHandler>);

    void ChangeHandler::ItChanged(int param, awl::String val)
    {
        pContext->logger->debug(_T("It has changed {} {}"), param, val);

        if (param == 2)
        {
            AWL_ASSERT(val == _T("temporary"));
        }

        changeHandled = true;
    }

    class Something : public awl::Observable<INotifySomethingChanged>
    {
    public:

        int It = 0;

        void SetIt(int it)
        {
            It = it;

            awl::String val = Formatter<int>::toString(It);

            awl::String val_copy = val;

            //Here val_copy is not temporary.
            notify(&INotifySomethingChanged::ItChanged, it, val_copy);

            AWL_ASSERT(val_copy == val);
        }

        void SetIt2(int it)
        {
            It = it;

            //This test demonstrates why notify should not use std::forward<Args>.
            notify(&INotifySomethingChanged::ItChanged, it, awl::String(_T("temporary")));
        }
    };
}

AWL_TEST(Observable_Events)
{
    Something something;

    AWL_ASSERT_EQUAL(0U, something.size());
    AWL_ASSERT(something.empty());

    {
        ChangeHandler handler1(context);
        ChangeHandler handler2(context);
        ChangeHandler handler3(context);

        something.subscribe(&handler1);
        something.subscribe(&handler2);
        something.subscribe(&handler3);

        AWL_ASSERT_EQUAL(3u, something.size());

        something.SetIt(1);
        something.SetIt2(2);

        AWL_ASSERTM_TRUE(handler1.changeHandled, _T("The observer has not been notified"));
        AWL_ASSERTM_TRUE(handler2.changeHandled, _T("The observer has not been notified"));
        AWL_ASSERTM_TRUE(handler3.changeHandled, _T("The observer has not been notified"));

        handler1.unsubscribeSelf();
        AWL_ASSERT_EQUAL(2u, something.size());

        something.unsubscribe(&handler2);
        AWL_ASSERT_EQUAL(1u, something.size());

        AWL_ASSERT(!something.empty());
    }

    AWL_ASSERT_EQUAL(0U, something.size());
    AWL_ASSERT(something.empty());
}

AWL_TEST(Observable_Move)
{
    Something something1;

    ChangeHandler handler1(context);
    ChangeHandler handler2(context);

    something1.subscribe(&handler1);
    something1.subscribe(&handler2);

    Something something2 = std::move(something1);

    something2.SetIt(5);

    AWL_ASSERTM_TRUE(handler1.changeHandled, _T("The observer has not been notified"));
    AWL_ASSERTM_TRUE(handler2.changeHandled, _T("The observer has not been notified"));

    handler1.changeHandled = false;
    handler2.changeHandled = false;

    Something something3;

    something3 = std::move(something2);

    something3.SetIt(7);

    AWL_ASSERTM_TRUE(handler1.changeHandled, _T("The observer has not been notified"));
    AWL_ASSERTM_TRUE(handler2.changeHandled, _T("The observer has not been notified"));
}

AWL_TEST(Observer_Move)
{
    Something something1;

    ChangeHandler handler1(context);
    ChangeHandler handler2(context);

    something1.subscribe(&handler1);
    something1.subscribe(&handler2);

    ChangeHandler handler1_copy = std::move(handler1);

    ChangeHandler handler2_copy(context);
    handler2_copy = std::move(handler2);
    
    something1.SetIt(5);

    AWL_ASSERTM_FALSE(handler1.included(), _T("Observer #1 is included"));
    AWL_ASSERTM_FALSE(handler2.included(), _T("Observer #2 is included"));
    AWL_ASSERTM_FALSE(handler1.changeHandled, _T("Observer #1 has been notified by a mistake."));
    AWL_ASSERTM_FALSE(handler2.changeHandled, _T("Observer #2 has been notified by a mistake"));

    AWL_ASSERTM_TRUE(handler1_copy.included(), _T("Observer #1 is not included"));
    AWL_ASSERTM_TRUE(handler2_copy.included(), _T("Observer #2 is not included"));
    AWL_ASSERTM_TRUE(handler1_copy.changeHandled, _T("Observer copy #1 has not been notified"));
    AWL_ASSERTM_TRUE(handler2_copy.changeHandled, _T("Observer copy #2 has not been notified"));
}

namespace
{
    struct Model
    {
        Something something;

        ChangeHandler handler1;
        ChangeHandler handler2;
        ChangeHandler handler3;

        void SetContext(const TestContext & c)
        {
            handler1.pContext = &c;
            handler2.pContext = &c;
        }

        Model()
        {
            something.subscribe(&handler1);
            something.subscribe(&handler2);
        }

        Model(const Model & other) = delete;
        Model(Model && other) = default;
        Model& operator = (const Model& other) = delete;
        Model& operator = (Model&& other) = default;
    };
}

AWL_TEST(Observable_ModelMove)
{
    Model m;
    m.SetContext(context);
    m.something.SetIt(5);

    AWL_ASSERTM_TRUE(m.handler1.included(), _T("Observer #1 is not included"));
    AWL_ASSERTM_TRUE(m.handler2.included(), _T("Observer #2 is not included"));
    AWL_ASSERTM_FALSE(m.handler3.included(), _T("Observer #3 is included"));
    AWL_ASSERTM_TRUE(m.handler1.changeHandled, _T("Observer copy #1 has not been notified"));
    AWL_ASSERTM_TRUE(m.handler2.changeHandled, _T("Observer copy #2 has not been notified"));

    m = {};
    m.SetContext(context);

    AWL_ASSERTM_TRUE(m.handler1.included(), _T("Observer #1 is not included"));
    AWL_ASSERTM_TRUE(m.handler2.included(), _T("Observer #2 is not included"));
    AWL_ASSERTM_FALSE(m.handler3.included(), _T("Observer #3 is included"));
    AWL_ASSERTM_FALSE(m.handler1.changeHandled, _T("Observer #1 has been notified by a mistake."));
    AWL_ASSERTM_FALSE(m.handler2.changeHandled, _T("Observer #2 has been notified by a mistake"));

    m.something.SetIt(7);

    AWL_ASSERTM_TRUE(m.handler1.changeHandled, _T("Observer copy #1 has not been notified"));
    AWL_ASSERTM_TRUE(m.handler2.changeHandled, _T("Observer copy #2 has not been notified"));
}

namespace
{
    class ChangeHandler2
    {
    public:

        ChangeHandler2(const TestContext & c) : context(c)
        {
        }

        void SomeHanderFunc(const awl::String & val)
        {
            context.logger->debug(_T("The value is: {}"), val);
        }

    private:

        const TestContext & context;
    };

    template<typename ...Params, typename ... Args>
    void WrongNotify(ChangeHandler2 & h, void (ChangeHandler2::*func)(Params ...), Args&&... args)
    {
        (h.*func)(std::forward<Args>(args) ...);
    }

    template<typename ...Params>
    static void WrongNotify2(ChangeHandler2 &, void (ChangeHandler2::*func)(Params ...), awl::String && val)
    {
        func(std::forward<const awl::String &&>(val));
    }
}

AWL_TEST(Observable_ForwardArgs)
{
    awl::String val = _T("String Value 1");

    ChangeHandler2 h(context);

    WrongNotify(h, &ChangeHandler2::SomeHanderFunc, val);
    WrongNotify(h, &ChangeHandler2::SomeHanderFunc, val);
    WrongNotify(h, &ChangeHandler2::SomeHanderFunc, val);

    WrongNotify(h, &ChangeHandler2::SomeHanderFunc, awl::String(_T("String Value 2")));
}

namespace
{
    struct IConditioncheck
    {
        virtual bool check(int value) = 0;
    };

    class ConditionHandler : public awl::Observer<IConditioncheck>
    {
    public:

        ConditionHandler(bool result, int* p_count, int* p_last_value)
            : m_result(result), pCount(p_count), pLastValue(p_last_value)
        {
        }

        bool check(int value) override
        {
            ++(*pCount);
            *pLastValue = value;
            return m_result;
        }

    private:

        bool m_result;
        int* pCount = nullptr;
        int* pLastValue = nullptr;
    };

    class ConditionObservable : public awl::Observable<IConditioncheck>
    {
    public:

        bool checkAll(int value)
        {
            return notifyWhileTrue(&IConditioncheck::check, value);
        }
    };
}

AWL_TEST(Observable_NotifyWhileTrue_StopsOnFalse)
{
    AWL_UNUSED_CONTEXT;

    ConditionObservable observable;

    int count1 = 0;
    int count2 = 0;
    int count3 = 0;
    int last1 = 0;
    int last2 = 0;
    int last3 = 0;

    ConditionHandler handler1(true, &count1, &last1);
    ConditionHandler handler2(false, &count2, &last2);
    ConditionHandler handler3(true, &count3, &last3);

    observable.subscribe(&handler1);
    observable.subscribe(&handler2);
    observable.subscribe(&handler3);

    const bool result = observable.checkAll(42);

    AWL_ASSERTM_FALSE(result, _T("notifyWhileTrue should stop at first false."));
    AWL_ASSERT_EQUAL(1, count1);
    AWL_ASSERT_EQUAL(1, count2);
    AWL_ASSERT_EQUAL(0, count3);
    AWL_ASSERT_EQUAL(42, last1);
    AWL_ASSERT_EQUAL(42, last2);
    AWL_ASSERT_EQUAL(0, last3);
}

AWL_TEST(Observable_NotifyWhileTrue_AllTrue)
{
    AWL_UNUSED_CONTEXT;

    ConditionObservable observable;

    int count1 = 0;
    int count2 = 0;
    int last1 = 0;
    int last2 = 0;

    ConditionHandler handler1(true, &count1, &last1);
    ConditionHandler handler2(true, &count2, &last2);

    observable.subscribe(&handler1);
    observable.subscribe(&handler2);

    const bool result = observable.checkAll(7);

    AWL_ASSERTM_TRUE(result, _T("notifyWhileTrue should return true when all observers return true."));
    AWL_ASSERT_EQUAL(1, count1);
    AWL_ASSERT_EQUAL(1, count2);
    AWL_ASSERT_EQUAL(7, last1);
    AWL_ASSERT_EQUAL(7, last2);
}

namespace
{
    using namespace std::chrono_literals;

    struct IAsyncNotify
    {
        virtual awl::Task<void> changed(int value) = 0;
    };

    class AsyncHandler : public awl::Observer<IAsyncNotify>
    {
    public:
        AsyncHandler(
            awl::coro::IDelayedExecutor& delayed_executor,
            int tag,
            std::vector<int>* p_events,
            bool defer = false) :
            m_delayedExecutor(delayed_executor),
            m_tag(tag),
            m_events(p_events),
            m_defer(defer)
        {
        }

        awl::Task<void> changed(int value) override
        {
            if (m_defer)
            {
                co_await awl::coro::DelayedAwaitable(m_delayedExecutor, 1ms);
            }

            m_events->push_back(m_tag * 1000 + value);
            co_return;
        }

    private:
        awl::coro::IDelayedExecutor& m_delayedExecutor;
        int m_tag = 0;
        std::vector<int>* m_events = nullptr;
        bool m_defer = false;
    };

    class AsyncObservable : public awl::Observable<IAsyncNotify>
    {
    public:
        awl::Task<int> setValueAsync(int value)
        {
            co_await notifyAsync(&IAsyncNotify::changed, value);
            co_return value;
        }
    };
}

AWL_TEST(Observable_NotifyAsync)
{
    AWL_UNUSED_CONTEXT;

    AsyncObservable observable;
    awl::testing::TimeQueue time_queue;
    std::vector<int> events;

    AsyncHandler handler1(time_queue, 1, &events, true);
    AsyncHandler handler2(time_queue, 2, &events, true);

    observable.subscribe(&handler1);
    observable.subscribe(&handler2);

    awl::Task<int> task = observable.setValueAsync(7);

    AWL_ASSERT(!task.is_ready());
    AWL_ASSERT_EQUAL(0u, events.size());

    time_queue.loop(1);

    AWL_ASSERT(!task.is_ready());
    AWL_ASSERT_EQUAL(1u, events.size());
    AWL_ASSERT_EQUAL(1007, events[0]);

    time_queue.loop();

    AWL_ASSERT(task.is_ready());
    AWL_ASSERT_EQUAL(7, task.get());

    AWL_ASSERT_EQUAL(2u, events.size());
    AWL_ASSERT_EQUAL(2007, events[1]);
}
