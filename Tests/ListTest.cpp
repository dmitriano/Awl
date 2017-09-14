#include <iostream>
#include <algorithm>

#include "Awl/QuickList.h"

#include "UnitTesting.h"

using namespace UnitTesting;

class LinkA : public awl::basic_quick_link<LinkA>
{
	typedef awl::basic_quick_link<LinkA> Base;

public:

	using Base::Base;
};

class LinkB : public awl::basic_quick_link<LinkB>
{
	typedef awl::basic_quick_link<LinkB> Base;

public:

	using Base::Base;
};

class Element : public LinkA, public LinkB, public awl::quick_link
{
public:

	Element(int val) : Value(val)
	{
		++elementCount;
	}

	~Element()
	{
		LinkA::safe_exclude();
		LinkB::safe_exclude();
		awl::quick_link::safe_exclude();

		--elementCount;
	}

	int Value = 0;

	static int elementCount;
};

int Element::elementCount = 0;

template <class DLink>
class ListHolder
{
	typedef awl::quick_list<Element, DLink> ELEMENT_LIST;

	ELEMENT_LIST list;

public:

	ListHolder()
	{
		Assert::IsTrue(list.empty());
		Assert::AreEqual(size_t(0), list.size());
		Assert::IsTrue(list.empty_or_contains_one());
		Assert::IsFalse(list.contains_one());

		list.push_back(new Element(1));

		Assert::IsFalse(list.empty());
		Assert::AreEqual(size_t(1), list.size());
		Assert::IsTrue(list.empty_or_contains_one());
		Assert::IsTrue(list.contains_one());

		list.push_back(new Element(2));
		
		Assert::IsFalse(list.empty());
		Assert::AreEqual(size_t(2), list.size());
		Assert::IsFalse(list.empty_or_contains_one());
		Assert::IsFalse(list.contains_one());

		list.push_front(new Element(0));

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

		for (typename ELEMENT_LIST::iterator i = list.begin(); i != list.end(); ++i)
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

		for (typename ELEMENT_LIST::const_iterator i = list.begin(); i != list.end(); ++i)
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

		for (typename ELEMENT_LIST::reverse_iterator i = list.rbegin(); i != list.rend(); ++i)
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

		for (typename ELEMENT_LIST::const_reverse_iterator i = list.rbegin(); i != list.rend(); ++i)
		{
			const Element * e = *i;

			std::cout << " " << e->Value;

			Assert::AreEqual(val--, e->Value);
		}

		std::cout << std::endl;
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
		list.push_back(new Element(4));

		Assert::AreEqual((size_t)(4), list.size());

		auto i = std::find_if(list.begin(), list.end(), [](const Element * e) -> bool { return e->Value == 2; });

		if (i == list.end())
		{
			throw TestException("Element 2 not found.");
		}

		list.insert(i, new Element(3));

		Assert::AreEqual((size_t)(5), list.size());

		PrintList();

		++i;
		
		Element * p_element_to_be_deleted = *i;
		
		list.erase(i++); //This only excludes the element from the list but not deletes it.

		Assert::IsFalse(p_element_to_be_deleted->DLink::included());
		
		delete p_element_to_be_deleted;

		Assert::AreEqual((size_t)(4), list.size());

		delete *(i++); //The element is excluded from the list automatically.

		Assert::AreEqual((size_t)(3), list.size());

		PrintList();
	}

    void MoveTest()
	{
		ELEMENT_LIST other_list;

		Assert::AreEqual((size_t)(3), list.size());

        other_list = std::move(list);

		Assert::AreEqual((size_t)(0), list.size());

		Assert::AreEqual((size_t)(3), other_list.size());

        list = std::move(other_list);

		Assert::AreEqual((size_t)(0), other_list.size());

		Assert::AreEqual((size_t)(3), list.size());

		PrintList();
	}

	~ListHolder()
	{
		while (!list.empty())
		{
			delete list.pop_front();
		}
	}
};

template <class DLink>
void TestLink()
{
	{
		ListHolder<DLink> holder;

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

		holder.InsertTest();

        holder.MoveTest();
	}

	Assert::AreEqual(0, Element::elementCount);
}

struct EleMent : awl::single_link
{
	int a = 25;
};

void TestSingleList()
{
	awl::single_list<EleMent> list;

	size_t count = 0;

	Assert::AreEqual(list.size(), count);

	list.push_front(new EleMent);

	Assert::AreEqual(list.size(), ++count);

	list.push_front(new EleMent);

	Assert::AreEqual(list.size(), ++count);

	list.push_front(new EleMent);

	Assert::AreEqual(list.size(), ++count);

	delete list.pop_front();

	Assert::AreEqual(list.size(), --count);

	delete list.pop_front();

	Assert::AreEqual(list.size(), --count);

	delete list.pop_front();

	Assert::AreEqual(list.size(), --count);

	list.push_front(new EleMent);

	Assert::AreEqual(list.size(), ++count);

	list.clear();

	Assert::AreEqual(list.size(), (size_t)0);
}

void TestList()
{
	TestLink<LinkA>();
	TestLink<LinkB>();
	TestLink<awl::quick_link>();
	TestSingleList();
}
