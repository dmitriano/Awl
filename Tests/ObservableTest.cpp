#include "Awl/Observable.h"
#include "Awl/String.h"
#include "Awl/Testing/UnitTest.h"
#include "Awl/Testing/Formatter.h"

using namespace awl::testing;

struct INotifySomethingChanged
{
    virtual void ItChanged(int param, awl::String val) = 0;
};

class ChangeHandler : public awl::Observer<INotifySomethingChanged>
{
public:

    ChangeHandler(const TestContext & c) : context(c)
    {
    }

    void ItChanged(int param, awl::String val) override;

    bool changeHandled = false;

private:

    const TestContext & context;
};

void ChangeHandler::ItChanged(int param, awl::String val)
{
    context.out << _T("It has changed ") << param << _T(" ") << val << std::endl;

    if (param == 2)
    {
        Assert::IsTrue(val == _T("temporary"));
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

        Assert::IsTrue(val_copy == val);
    }

    void SetIt2(int it)
    {
        It = it;

        //This test demonstrates why Notify should not use std::forward<Args>.
        Notify(&INotifySomethingChanged::ItChanged, it, awl::String(_T("temporary")));
    }
};

AWL_TEST(Observable_Events)
{
    Something something;

    Assert::AreEqual(0, something.size());
    Assert::IsTrue(something.empty());

    {
        ChangeHandler handler1(context);
        ChangeHandler handler2(context);
        ChangeHandler handler3(context);

        something.Subscribe(&handler1);
        something.Subscribe(&handler2);
        something.Subscribe(&handler3);

        Assert::AreEqual(3u, something.size());

        something.SetIt(1);
        something.SetIt2(2);

        Assert::IsTrue(handler1.changeHandled, _T("The observer has not been notified"));
        Assert::IsTrue(handler2.changeHandled, _T("The observer has not been notified"));
        Assert::IsTrue(handler3.changeHandled, _T("The observer has not been notified"));

        handler1.UnsubscribeSelf();
        Assert::AreEqual(2u, something.size());

        something.Unsubscribe(&handler2);
        Assert::AreEqual(1u, something.size());

        Assert::IsTrue(!something.empty());
    }

    Assert::AreEqual(0, something.size());
    Assert::IsTrue(something.empty());
}

AWL_TEST(Observable_Move)
{
    Something something1;

    ChangeHandler handler1(context);

    ChangeHandler handler2(context);

    something1.Subscribe(&handler1);

    something1.Subscribe(&handler2);

    Something something2 = std::move(something1);

    something2.SetIt(5);

    Assert::IsTrue(handler1.changeHandled, _T("The observer has not been notified"));
    Assert::IsTrue(handler2.changeHandled, _T("The observer has not been notified"));

    handler1.changeHandled = false;
    handler2.changeHandled = false;

    Something something3;

    something3 = std::move(something2);

    something3.SetIt(7);

    Assert::IsTrue(handler1.changeHandled, _T("The observer has not been notified"));
    Assert::IsTrue(handler2.changeHandled, _T("The observer has not been notified"));
}
