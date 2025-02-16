/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <algorithm>

#include "Awl/QuickList.h"
#include "Awl/Testing/UnitTest.h"

#include <ranges>

using namespace awl::testing;

namespace
{
    class LinkA : public awl::basic_quick_link<LinkA>
    {
        using Base = awl::basic_quick_link<LinkA>;

    public:

        using Base::Base;
    };

    class LinkB : public awl::basic_quick_link<LinkB>
    {
        using Base = awl::basic_quick_link<LinkB>;

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
        using ELEMENT_LIST = awl::quick_list<Element, DLink>;

        //Check if it satisfies the concept std::ranges::range.
        static_assert(std::ranges::range<ELEMENT_LIST>);

        ELEMENT_LIST list;

        const awl::testing::TestContext & context;

        class Printer
        {
        public:

            Printer(const awl::testing::TestContext & test_context) : context(test_context)
            {
                context.out << _T("The list content is:");
            }

            ~Printer()
            {
                context.out << std::endl;
            }

            void PrintElement(const Element * e) const
            {
                context.out << _T(" ") << e->Value;
            }

        private:

            const awl::testing::TestContext & context;
        };

    public:

        ListHolder(const awl::testing::TestContext & test_context) : context(test_context)
        {
            AWL_ASSERT(list.empty());
            AWL_ASSERT_EQUAL(size_t(0), list.size());
            AWL_ASSERT(list.empty_or_contains_one());
            AWL_ASSERT_FALSE(list.contains_one());

            list.push_back(new Element(1));

            AWL_ASSERT_FALSE(list.empty());
            AWL_ASSERT_EQUAL(size_t(1), list.size());
            AWL_ASSERT(list.empty_or_contains_one());
            AWL_ASSERT(list.contains_one());

            list.push_back(new Element(2));

            AWL_ASSERT_FALSE(list.empty());
            AWL_ASSERT_EQUAL(size_t(2), list.size());
            AWL_ASSERT_FALSE(list.empty_or_contains_one());
            AWL_ASSERT_FALSE(list.contains_one());

            list.push_front(new Element(0));

            AWL_ASSERT_EQUAL(size_t(3), list.size());
        }

        void AddRemoveTest()
        {
            list.push_front(new Element(-1));
            list.push_back(new Element(3));

            AWL_ASSERT_EQUAL(size_t(5), list.size());

            delete list.pop_front();
            delete list.pop_back();

            AWL_ASSERT_EQUAL(size_t(3), list.size());
        }

        void AutoRemoveTest()
        {
            auto * p_elem = new Element(-1);

            list.push_front(p_elem);

            AWL_ASSERT_EQUAL(size_t(4), list.size());

            delete p_elem;

            AWL_ASSERT_EQUAL(size_t(3), list.size());
        }

        void PrintList()
        {
            int val = 0;

            Printer printer(context);

            for (Element * e : list)
            {
                printer.PrintElement(e);

                AWL_ASSERT_EQUAL(val++, e->Value);
            }
        }

        void PrintListAuto()
        {
            int val = 0;

            Printer printer(context);

            for (auto e : list)
            {
                printer.PrintElement(e);

                AWL_ASSERT_EQUAL(val++, e->Value);
            }
        }

        void PrintListConst() const
        {
            int val = 0;

            Printer printer(context);

            for (const Element * e : list)
            {
                printer.PrintElement(e);

                AWL_ASSERT_EQUAL(val++, e->Value);
            }
        }

        void PrintListIter()
        {
            Printer printer(context);

            int val = 0;

            for (typename ELEMENT_LIST::iterator i = list.begin(); i != list.end(); ++i)
            {
                Element * e = *i;

                printer.PrintElement(e);

                AWL_ASSERT_EQUAL(val++, e->Value);
            }
        }

        void PrintListIterConst()
        {
            Printer printer(context);

            int val = 0;

            for (typename ELEMENT_LIST::const_iterator i = list.begin(); i != list.end(); ++i)
            {
                const Element * e = *i;

                printer.PrintElement(e);

                AWL_ASSERT_EQUAL(val++, e->Value);
            }
        }

        void PrintListReverseIter()
        {
            Printer printer(context);

            int val = 2;

            for (typename ELEMENT_LIST::reverse_iterator i = list.rbegin(); i != list.rend(); ++i)
            {
                Element * e = *i;

                printer.PrintElement(e);

                AWL_ASSERT_EQUAL(val--, e->Value);
            }
        }

        void PrintListReverseIterConst()
        {
            Printer printer(context);

            int val = 2;

            for (typename ELEMENT_LIST::const_reverse_iterator i = list.rbegin(); i != list.rend(); ++i)
            {
                const Element * e = *i;

                printer.PrintElement(e);

                AWL_ASSERT_EQUAL(val--, e->Value);
            }
        }

        void ConstAlgorithmTest() const
        {
            auto i = std::find_if(list.begin(), list.end(), [](const Element * e) -> bool { return e->Value == 1; });

            if (i == list.end())
            {
                AWL_FAILM(_T("Element 1 not found."));
            }

            context.out << _T("The found element is: ") << i->Value << std::endl;

            i = std::find_if(list.begin(), list.end(), [](const Element * e) -> bool { return e->Value == 25; });

            if (i != list.end())
            {
                AWL_FAILM(_T("Non-existing element 25 found."));
            }

            Printer printer(context);

            std::for_each(list.begin(), list.end(), [&printer](const Element * e) { printer.PrintElement(e); });
        }

        void InsertTest()
        {
            list.push_back(new Element(4));

            AWL_ASSERT_EQUAL((size_t)(4), list.size());

            auto i = std::find_if(list.begin(), list.end(), [](const Element * e) -> bool { return e->Value == 2; });

            if (i == list.end())
            {
                AWL_FAILM(_T("Element 2 not found."));
            }

            list.insert(i, new Element(3));

            AWL_ASSERT_EQUAL((size_t)(5), list.size());

            PrintList();

            ++i;

            Element * p_element_to_be_deleted = *i;

            list.erase(i++); //This only excludes the element from the list but not deletes it.

            AWL_ASSERT_FALSE(p_element_to_be_deleted->DLink::included());

            delete p_element_to_be_deleted;

            AWL_ASSERT_EQUAL((size_t)(4), list.size());

            delete *(i++); //The element is excluded from the list automatically.

            AWL_ASSERT_EQUAL((size_t)(3), list.size());

            PrintList();
        }

        void MoveTest()
        {
            ELEMENT_LIST other_list;

            AWL_ASSERT_EQUAL((size_t)(3), list.size());

            other_list = std::move(list);

            AWL_ASSERT_EQUAL((size_t)(0), list.size());

            AWL_ASSERT_EQUAL((size_t)(3), other_list.size());

            list = std::move(other_list);

            AWL_ASSERT_EQUAL((size_t)(0), other_list.size());

            AWL_ASSERT_EQUAL((size_t)(3), list.size());

            PrintList();
        }

        void PushBackTest()
        {
            AWL_ASSERT_EQUAL((size_t)(3), list.size());

            ELEMENT_LIST other_list;

            other_list.push_back(list);

            AWL_ASSERT_EQUAL((size_t)(0), list.size());

            AWL_ASSERT_EQUAL((size_t)(3), other_list.size());

            list.push_back(other_list);

            AWL_ASSERT_EQUAL((size_t)(0), other_list.size());

            AWL_ASSERT_EQUAL((size_t)(3), list.size());

            PrintList();
        }

        void BidirectionalTest()
        {
            auto i = list.end();

            AWL_ASSERT_EQUAL(2, (--i)->Value);
            AWL_ASSERT_EQUAL(1, (--i)->Value);
            AWL_ASSERT_EQUAL(0, (--i)->Value);

            AWL_ASSERT(i == list.begin());

            AWL_ASSERT_EQUAL(0, (i++)->Value);
            AWL_ASSERT_EQUAL(1, (i++)->Value);
            AWL_ASSERT_EQUAL(2, (i++)->Value);

            AWL_ASSERT(i == list.end());
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
    static void TestLinkFunc(const awl::testing::TestContext & context)
    {
        {
            ListHolder<DLink> holder(context);

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

            holder.PushBackTest();

            holder.BidirectionalTest();
        }

        AWL_ASSERT_EQUAL(0, Element::elementCount);
    }

    struct EleMent : awl::single_link
    {
        int a = 25;
    };
}

AWL_TEST(List)
{
    TestLinkFunc<LinkA>(context);
    TestLinkFunc<LinkB>(context);
    TestLinkFunc<awl::quick_link>(context);
}

AWL_TEST(List_SingleList)
{
    AWL_UNUSED_CONTEXT;

    awl::single_list<EleMent> list;

    size_t count = 0;

    AWL_ASSERT_EQUAL(list.size(), count);

    list.push_front(new EleMent);

    AWL_ASSERT_EQUAL(list.size(), ++count);

    list.push_front(new EleMent);

    AWL_ASSERT_EQUAL(list.size(), ++count);

    list.push_front(new EleMent);

    AWL_ASSERT_EQUAL(list.size(), ++count);

    delete list.pop_front();

    AWL_ASSERT_EQUAL(list.size(), --count);

    delete list.pop_front();

    AWL_ASSERT_EQUAL(list.size(), --count);

    delete list.pop_front();

    AWL_ASSERT_EQUAL(list.size(), --count);

    list.push_front(new EleMent);

    AWL_ASSERT_EQUAL(list.size(), ++count);

    auto first = list.front();

    list.clear();

    AWL_ASSERT_EQUAL(list.size(), (size_t)0);

    delete first;
}
