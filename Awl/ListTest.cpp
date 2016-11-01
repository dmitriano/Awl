#include <iostream>
#include <algorithm>

#include "Awl/QuickList.h"

#include "UnitTesting.h"

using namespace UnitTesting;

class Element : public awl::quick_link<Element>
{
public:

	Element(int val) : Value(val)
	{
	}

	~Element()
	{
		if (included())
		{
			exclude();
		}
	}

	int Value = 0;
};

typedef awl::quick_list<Element> ELEMENT_LIST;

class ListHolder
{
	ELEMENT_LIST list;

public:

	ListHolder()
	{
		list.push_back(new Element(0));
		list.push_back(new Element(1));
		list.push_back(new Element(2));

		Assert::AreEqual(size_t(3), list.size());
	}

	void AddRemoveTest()
	{
		list.push_front(new Element(-1));
		list.push_back(new Element(3));

		Assert::AreEqual(size_t(5), list.size());

		delete list.pop_front();
		delete list.pop_back();

		Assert::AreEqual(size_t(3), list.size());
	}

	void AutoRemoveTest()
	{
		auto * p_elem = new Element(-1);

		list.push_front(p_elem);

		Assert::AreEqual(size_t(4), list.size());

		delete p_elem;

		Assert::AreEqual(size_t(3), list.size());
	}

	void PrintList()
	{
		int val = 0;
		
		std::cout << _T("The list content is:");
		
		for (Element * e : list)
		{
			std::cout << " " << e->Value;

			Assert::AreEqual(val++, e->Value);
		}

		std::cout << std::endl;
	}

	void PrintListAuto()
	{
		int val = 0;

		std::cout << _T("The list content is:");

		for (auto e : list)
		{
			std::cout << " " << e->Value;

			Assert::AreEqual(val++, e->Value);
		}

		std::cout << std::endl;
	}

	void PrintListConst() const
	{
		int val = 0;

		std::cout << _T("The list content is:");

		for (const Element * e : list)
		{
			std::cout << " " << e->Value;

			Assert::AreEqual(val++, e->Value);
		}

		std::cout << std::endl;
	}

	void PrintListIter()
	{
		std::cout << _T("The list content is:");

		int val = 0;

		for (ELEMENT_LIST::iterator i = list.begin(); i != list.end(); ++i)
		{
			Element * e = *i;
			
			std::cout << " " << e->Value;

			Assert::AreEqual(val++, e->Value);
		}

		std::cout << std::endl;
	}

	void PrintListIterConst()
	{
		std::cout << _T("The list content is:");

		int val = 0;

		for (ELEMENT_LIST::const_iterator i = list.begin(); i != list.end(); ++i)
		{
			const Element * e = *i;

			std::cout << " " << e->Value;

			Assert::AreEqual(val++, e->Value);
		}

		std::cout << std::endl;
	}

	void PrintListReverseIter()
	{
		std::cout << _T("The list content is:");

		int val = 2;

		for (ELEMENT_LIST::reverse_iterator i = list.rbegin(); i != list.rend(); ++i)
		{
			Element * e = *i;

			std::cout << " " << e->Value;

			Assert::AreEqual(val--, e->Value);
		}

		std::cout << std::endl;
	}

	void PrintListReverseIterConst()
	{
		std::cout << _T("The list content is:");

		int val = 2;

		for (ELEMENT_LIST::const_reverse_iterator i = list.rbegin(); i != list.rend(); ++i)
		{
			const Element * e = *i;

			std::cout << " " << e->Value;

			Assert::AreEqual(val--, e->Value);
		}

		std::cout << std::endl;
	}

	//! Conversion from non-const inerator to const interator.
	ELEMENT_LIST::const_iterator IterConstCastTest()
	{
		ELEMENT_LIST::const_iterator i = list.begin();

		ELEMENT_LIST::const_reverse_iterator r_i = list.rbegin();
	}

	void ConstAlgorithmTest() const
	{
		//This code does not complile with MSVC in Debug configuration.
		
		auto i = std::find_if(list.begin(), list.end(), [](const Element * e) -> bool { return e->Value == 1; });

		if (i == list.end())
		{
			throw TestException("Element 1 not found.");
		}

		std::cout << _T("The found element is: ") << i->Value << std::endl;

		i = std::find_if(list.begin(), list.end(), [](const Element * e) -> bool { return e->Value == 25; });

		if (i != list.end())
		{
			throw TestException("Non-existing element 25 found.");
		}

		std::cout << _T("The list content is:");

		std::for_each(list.begin(), list.end(), [](const Element * e) { std::cout << " " << e->Value; });

		std::cout << std::endl;
	}

	void InsertTest()
	{

	}

	~ListHolder()
	{
		while (!list.empty())
		{
			delete list.pop_front();
		}
	}
};

void TestList()
{
	ListHolder holder;

	holder.AddRemoveTest();

	holder.AutoRemoveTest();

	holder.PrintList();

	holder.PrintListAuto();

	holder.PrintListConst();

	holder.PrintListIter();

	holder.PrintListIterConst();

	holder.PrintListReverseIterConst();

	holder.PrintListReverseIter();

	holder.ConstAlgorithmTest();
}