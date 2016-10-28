#include <iostream>

#include "Awl/Observable.h"

struct INotifySomethingChanged
{
	virtual void ItChanged(int param) = 0;
};

class ChangeHandler : public awl::Observer<INotifySomethingChanged>
{
public:

	virtual void ItChanged(int param) override;
};

void ChangeHandler::ItChanged(int param)
{
	std::cout << "It has changed " << param << std::endl;
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

	handler.UnsubscribeSelf();
}