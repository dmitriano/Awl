/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <algorithm>

#include "Awl/QuickList.h"
#include "Awl/Testing/UnitTest.h"
#include "Awl/RangeUtil.h"
#include "Awl/String.h"

#include <ranges>
#include <functional>

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

    static_assert(!std::is_copy_assignable_v<LinkA>);
    static_assert(!std::is_copy_constructible_v<LinkA>);
    static_assert(!std::is_copy_assignable_v<LinkB>);
    static_assert(!std::is_copy_constructible_v<LinkB>);
    static_assert(!std::is_copy_assignable_v<awl::quick_link>);
    static_assert(!std::is_copy_constructible_v<awl::quick_link>);

    static_assert(std::is_move_assignable_v<LinkA>);
    static_assert(std::is_move_constructible_v<LinkA>);
    static_assert(std::is_move_assignable_v<LinkB>);
    static_assert(std::is_move_constructible_v<LinkB>);
    static_assert(std::is_move_assignable_v<awl::quick_link>);
    static_assert(std::is_move_constructible_v<awl::quick_link>);

    class CompositeLink :
        public LinkA,
        public LinkB,
        public awl::quick_link
    {};

    class Element : public CompositeLink
    {
    public:

        Element(int val) : Value(val)
        {
            ++elementCount;
        }

        Element(Element&& other) : CompositeLink(std::move(other))
        {
            ++elementCount;
        }

        Element& operator = (Element&& other)
        {
            *(static_cast<CompositeLink*>(this)) = std::move(other);

            ++elementCount;

            return *this;
        }

        ~Element()
        {
            --elementCount;
        }

        int Value = 0;

        static int elementCount;
    };

    static_assert(!std::is_copy_assignable_v<Element>);
    static_assert(!std::is_copy_constructible_v<Element>);
    static_assert(std::is_move_assignable_v<Element>);
    static_assert(std::is_move_constructible_v<Element>);

    int Element::elementCount = 0;

    template <class DLink>
    class ListHolder
    {
        using ElementList = awl::quick_list<Element, DLink>;

        //Check if it satisfies the concept std::ranges::range.
        static_assert(awl::range_over<ElementList, Element*>);

        ElementList list;

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

            for (typename ElementList::iterator i = list.begin(); i != list.end(); ++i)
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

            for (typename ElementList::const_iterator i = list.begin(); i != list.end(); ++i)
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

            for (typename ElementList::reverse_iterator i = list.rbegin(); i != list.rend(); ++i)
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

            for (typename ElementList::const_reverse_iterator i = list.rbegin(); i != list.rend(); ++i)
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
            ElementList other_list;

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

            ElementList other_list;

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

    struct SingleElement : public awl::single_link
    {
        SingleElement(std::string name, int val) :
            m_name(std::move(name)),
            m_val(val)
        {
        }

        const char* name() const { return m_name.c_str(); }

        int value() const { return m_val; }

        std::string m_name;

        int m_val;
    };

    static_assert(!std::is_copy_assignable_v<SingleElement>);
    static_assert(!std::is_copy_constructible_v<SingleElement>);
    static_assert(!std::is_move_assignable_v<SingleElement>);
    static_assert(!std::is_move_constructible_v<SingleElement>);

    using SingleList = awl::single_list<SingleElement>;

    struct SimpleElement : SingleElement
    {
        SimpleElement() : SingleElement(awl::aformat() << m_count, m_count)
        {
            ++m_count;
        }

        static inline int m_count = 0;
    };

    static_assert(!std::is_copy_assignable_v<SimpleElement>);
    static_assert(!std::is_copy_constructible_v<SimpleElement>);
    static_assert(!std::is_move_assignable_v<SimpleElement>);
    static_assert(!std::is_move_constructible_v<SimpleElement>);

    static_assert(std::default_initializable<SingleList::iterator>);
}

AWL_TEST(List)
{
    TestLinkFunc<LinkA>(context);
    TestLinkFunc<LinkB>(context);
    TestLinkFunc<awl::quick_link>(context);
}

AWL_TEST(List_SingleListPushPop)
{
    AWL_UNUSED_CONTEXT;

    awl::single_list<SimpleElement> list;

    size_t count = 0;

    AWL_ASSERT_EQUAL(list.size(), count);

    list.push_front(new SimpleElement);

    AWL_ASSERT_EQUAL(list.size(), ++count);

    list.push_front(new SimpleElement);

    AWL_ASSERT_EQUAL(list.size(), ++count);

    list.push_front(new SimpleElement);

    AWL_ASSERT_EQUAL(list.size(), ++count);

    delete list.pop_front();

    AWL_ASSERT_EQUAL(list.size(), --count);

    delete list.pop_front();

    AWL_ASSERT_EQUAL(list.size(), --count);

    delete list.pop_front();

    AWL_ASSERT_EQUAL(list.size(), --count);

    list.push_front(new SimpleElement);

    AWL_ASSERT_EQUAL(list.size(), ++count);

    auto first = list.front();

    list.clear();

    AWL_ASSERT_EQUAL(list.size(), (size_t)0);

    delete first;
}

AWL_TEST(List_SingleListIncluded)
{
    AWL_UNUSED_CONTEXT;

    awl::single_list<SimpleElement> list;

    SimpleElement a;

    AWL_ASSERT(!a.included());

    list.push_front(&a);

    AWL_ASSERT(a.included());

    list.pop_front();

    AWL_ASSERT(!a.included());
}

AWL_TEST(List_FindIf)
{
    AWL_UNUSED_CONTEXT;

    SingleElement a{ "a", 1 };
    SingleElement b{ "B", 2 };
    SingleElement c{ "c", 3 };

    SingleList list;

    auto pred = std::bind(awl::CStringInsensitiveEqual<char>(), "b", std::placeholders::_1);

    list.push_front(&c);

    auto i = std::ranges::find_if(list.begin(), list.end(), pred, std::mem_fn(&SingleElement::name));

    AWL_ASSERT(i == list.end());

    list.push_front(&b);
    list.push_front(&a);

    i = std::ranges::find_if(list.begin(), list.end(), pred, std::mem_fn(&SingleElement::name));

    AWL_ASSERT(i != list.end());

    AWL_ASSERT(*i == &b);

    auto less = std::bind(awl::CStringInsensitiveLess<char>(), std::placeholders::_1, "b");

    auto range = list | std::views::filter([](auto* elem) { return elem->value() < 3; }) |
        std::views::transform(std::mem_fn(&SingleElement::name)) |
        std::views::filter(less);

    const std::size_t count = std::ranges::distance(range);

    AWL_ASSERT_EQUAL(1u, count);

    const char* name = *range.begin();

    AWL_ASSERT(awl::StrCmp("a", name) == 0);

    AWL_ASSERT_EQUAL(a.name(), name);
}

AWL_TEST(List_Destructor1)
{
    AWL_UNUSED_CONTEXT;

    {
        Element a(1);

        {
            Element b(2);

            {
                awl::quick_list<Element, LinkA> list;

                list.push_back(&a);
                list.push_back(&b);

                AWL_ASSERT_EQUAL(2u, list.size());
            }

            AWL_ASSERT(a.LinkA::included());
            AWL_ASSERT(b.LinkA::included());

            AWL_ASSERT(a.LinkA::predecessor() == static_cast<LinkA*>(&b));
            AWL_ASSERT(a.LinkA::successor() == static_cast<LinkA*>(&b));
        }

        AWL_ASSERT(a.LinkA::included());

        AWL_ASSERT(a.LinkA::predecessor() == static_cast<LinkA*>(&a));
        AWL_ASSERT(a.LinkA::successor() == static_cast<LinkA*>(&a));
    }
}

AWL_TEST(List_Destructor2)
{
    AWL_UNUSED_CONTEXT;

    {
        awl::quick_list<Element, LinkA> list;

        {
            Element a(1);

            {
                Element b(2);

                list.push_back(&a);
                list.push_back(&b);

                AWL_ASSERT_EQUAL(2u, list.size());
            }

            AWL_ASSERT(a.LinkA::included());

            AWL_ASSERT_EQUAL(1u, list.size());
        }

        AWL_ASSERT(list.empty());
    }
}

AWL_TEST(List_MovableElement)
{
    AWL_UNUSED_CONTEXT;

    awl::quick_list<Element, LinkA> list;

    Element a(1);

    list.push_back(&a);

    AWL_ASSERT(a.LinkA::included());

    AWL_ASSERT_EQUAL(1u, list.size());

    Element b = std::move(a);

    static_assert(&b != &a);

    AWL_ASSERT(!a.LinkA::included());
    AWL_ASSERT(b.LinkA::included());

    AWL_ASSERT_EQUAL(1u, list.size());
}
