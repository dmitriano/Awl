/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Observable.h"
#include "Awl/String.h"
#include "Awl/Testing/UnitTest.h"
#include "Awl/Testing/Formatter.h"

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

    void ChangeHandler::ItChanged(int param, awl::String val)
    {
        pContext->out << _T("It has changed ") << param << _T(" ") << val << std::endl;

        if (param == 2)
        {
            AWT_ASSERT(val == _T("temporary"));
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

            awl::String val = Formatter<int>::ToString(It);

            awl::String val_copy = val;

            //Here val_copy is not temporary.
            Notify(&INotifySomethingChanged::ItChanged, it, val_copy);

            AWT_ASSERT(val_copy == val);
        }

        void SetIt2(int it)
        {
            It = it;

            //This test demonstrates why Notify should not use std::forward<Args>.
            Notify(&INotifySomethingChanged::ItChanged, it, awl::String(_T("temporary")));
        }
    };
}

AWT_TEST(Observable_Events)
{
    Something something;

    AWT_ASSERT_EQUAL(0U, something.size());
    AWT_ASSERT(something.empty());

    {
        ChangeHandler handler1(context);
        ChangeHandler handler2(context);
        ChangeHandler handler3(context);

        something.Subscribe(&handler1);
        something.Subscribe(&handler2);
        something.Subscribe(&handler3);

        AWT_ASSERT_EQUAL(3u, something.size());

        something.SetIt(1);
        something.SetIt2(2);

        AWT_ASSERTM_TRUE(handler1.changeHandled, _T("The observer has not been notified"));
        AWT_ASSERTM_TRUE(handler2.changeHandled, _T("The observer has not been notified"));
        AWT_ASSERTM_TRUE(handler3.changeHandled, _T("The observer has not been notified"));

        handler1.UnsubscribeSelf();
        AWT_ASSERT_EQUAL(2u, something.size());

        something.Unsubscribe(&handler2);
        AWT_ASSERT_EQUAL(1u, something.size());

        AWT_ASSERT(!something.empty());
    }

    AWT_ASSERT_EQUAL(0U, something.size());
    AWT_ASSERT(something.empty());
}

AWT_TEST(Observable_Move)
{
    Something something1;

    ChangeHandler handler1(context);
    ChangeHandler handler2(context);

    something1.Subscribe(&handler1);
    something1.Subscribe(&handler2);

    Something something2 = std::move(something1);

    something2.SetIt(5);

    AWT_ASSERTM_TRUE(handler1.changeHandled, _T("The observer has not been notified"));
    AWT_ASSERTM_TRUE(handler2.changeHandled, _T("The observer has not been notified"));

    handler1.changeHandled = false;
    handler2.changeHandled = false;

    Something something3;

    something3 = std::move(something2);

    something3.SetIt(7);

    AWT_ASSERTM_TRUE(handler1.changeHandled, _T("The observer has not been notified"));
    AWT_ASSERTM_TRUE(handler2.changeHandled, _T("The observer has not been notified"));
}

AWT_TEST(Observer_Move)
{
    Something something1;

    ChangeHandler handler1(context);
    ChangeHandler handler2(context);

    something1.Subscribe(&handler1);
    something1.Subscribe(&handler2);

    ChangeHandler handler1_copy = std::move(handler1);

    ChangeHandler handler2_copy(context);
    handler2_copy = std::move(handler2);
    
    something1.SetIt(5);

    AWT_ASSERTM_FALSE(handler1.included(), _T("Observer #1 is included"));
    AWT_ASSERTM_FALSE(handler2.included(), _T("Observer #2 is included"));
    AWT_ASSERTM_FALSE(handler1.changeHandled, _T("Observer #1 has been notified by a mistake."));
    AWT_ASSERTM_FALSE(handler2.changeHandled, _T("Observer #2 has been notified by a mistake"));

    AWT_ASSERTM_TRUE(handler1_copy.included(), _T("Observer #1 is not included"));
    AWT_ASSERTM_TRUE(handler2_copy.included(), _T("Observer #2 is not included"));
    AWT_ASSERTM_TRUE(handler1_copy.changeHandled, _T("Observer copy #1 has not been notified"));
    AWT_ASSERTM_TRUE(handler2_copy.changeHandled, _T("Observer copy #2 has not been notified"));
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
            something.Subscribe(&handler1);
            something.Subscribe(&handler2);
        }

        Model(const Model & other) = delete;
        Model(Model && other) = default;
        Model& operator = (const Model& other) = delete;
        Model& operator = (Model&& other) = default;
    };
}

AWT_TEST(Observable_ModelMove)
{
    Model m;
    m.SetContext(context);
    m.something.SetIt(5);

    AWT_ASSERTM_TRUE(m.handler1.included(), _T("Observer #1 is not included"));
    AWT_ASSERTM_TRUE(m.handler2.included(), _T("Observer #2 is not included"));
    AWT_ASSERTM_FALSE(m.handler3.included(), _T("Observer #3 is included"));
    AWT_ASSERTM_TRUE(m.handler1.changeHandled, _T("Observer copy #1 has not been notified"));
    AWT_ASSERTM_TRUE(m.handler2.changeHandled, _T("Observer copy #2 has not been notified"));

    m = {};
    m.SetContext(context);

    AWT_ASSERTM_TRUE(m.handler1.included(), _T("Observer #1 is not included"));
    AWT_ASSERTM_TRUE(m.handler2.included(), _T("Observer #2 is not included"));
    AWT_ASSERTM_FALSE(m.handler3.included(), _T("Observer #3 is included"));
    AWT_ASSERTM_FALSE(m.handler1.changeHandled, _T("Observer #1 has been notified by a mistake."));
    AWT_ASSERTM_FALSE(m.handler2.changeHandled, _T("Observer #2 has been notified by a mistake"));

    m.something.SetIt(7);

    AWT_ASSERTM_TRUE(m.handler1.changeHandled, _T("Observer copy #1 has not been notified"));
    AWT_ASSERTM_TRUE(m.handler2.changeHandled, _T("Observer copy #2 has not been notified"));
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
            context.out << _T("The value is: ") << val << std::endl;
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

AWT_TEST(Observable_ForwardArgs)
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
    class X
    {
    public:
        X() {};
        X(const X&) { called = "copy"; };
        X(X&&) { called = "move"; };

        std::string called;
    };
}

AWT_TEST(Observable_ConstMove)
{
    AWT_UNUSED_CONTEXT;

    const X x1;
    X x2 = std::move(x1);

    AWT_ASSERT(x1.called == "");
    AWT_ASSERT(x2.called == "copy");
}
