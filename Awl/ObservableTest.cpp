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

		Notify(&INotifySomethingChanged::ItChanged, 25);
	}
};

void TestObservable()
{
	Something something;

	ChangeHandler handler;

	something.Subscribe(&handler);

	something.SetIt(3);

	Assert::IsTrue(handler.changeHandled, _T("The observer has not been notified"));

	handler.UnsubscribeSelf();
}