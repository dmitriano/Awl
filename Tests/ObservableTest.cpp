#include <iostream>

#include "Awl/Observable.h"

#include "UnitTesting.h"

using namespace UnitTesting;

struct INotifySomethingChanged
{
    virtual void ItChanged(int param) = 0;
};

class ChangeHandler : public awl::Observer<INotifySomethingChanged>
{
public:

    virtual void ItChanged(int param) override;

    bool changeHandled = false;
};

void ChangeHandler::ItChanged(int param)
{
    std::cout << _T("It has changed ") << param << std::endl;

    changeHandled = true;
}

class Something : public awl::Observable<INotifySomethingChanged>
{
public:

    int It = 0;

    void SetIt(int it)
    {
        It = it;

        Notify(&INotifySomethingChanged::ItChanged, it);
    }
};

static void TestEvents()
{
    Something something;

    ChangeHandler handler;

    something.Subscribe(&handler);

    something.SetIt(3);

    Assert::IsTrue(handler.changeHandled, _T("The observer has not been notified"));

    handler.UnsubscribeSelf();
}

static void TestMove()
{
    Something something1;

    ChangeHandler handler1;

    ChangeHandler handler2;

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

void TestObservable()
{
    TestEvents();

    TestMove();
}
