/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/UniquePtr.h"

#include "Awl/Testing/UnitTest.h"
#include "Awl/String.h"
#include "Awl/StringFormat.h"

#include <functional>

using namespace awl::testing;

namespace
{
    static int count = 0;
    
    struct A
    {
        A()
        {
            ++count;
        }

        std::function<void()> func;

        awl::String val;

        ~A()
        {
            func();

            --count;
        }
    };
}

//A similar code with std::unique_ptr results in a segmentation fault.
AWL_TEST(UniquePtrBegingDestoyed)
{
    AWL_UNUSED_CONTEXT;

    awl::unique_ptr<A> p_a = awl::make_unique<A>();

    AWL_ASSERT_EQUAL(1, count);

    p_a->val = _T("beging destoyed");
    p_a->func = [&p_a, context]() { context.logger.debug(awl::format() << p_a->val); };
    p_a = {};

    AWL_ASSERT_EQUAL(0, count);
}

namespace
{
    template <class T>
    class TableColumn
    {
    public:

        virtual void f() const = 0;

        virtual ~TableColumn() = default;
    };

    template <class T>
    struct ColumnDefinition
    {
        ColumnDefinition() = default;

        ColumnDefinition(std::string t, std::string n, TableColumn<T>* p = nullptr) :
            type(std::move(t)),
            name(std::move(n)),
            col(p)
        {
        }

        std::string type;
        std::string name;
        std::unique_ptr<TableColumn<T>> col;
    };

    struct ColumnDescriptor
    {
        ColumnDefinition<int> def;
        std::optional<std::string> prop;
    };
}

AWL_TEST(UniquePtrMove)
{
    AWL_UNUSED_CONTEXT;

    ColumnDefinition<int> d1;

    ColumnDefinition<int> d2 = std::move(d1);

    ColumnDescriptor desc;

    desc.def = std::move(d2);

    std::vector<ColumnDescriptor> cols;

    cols.push_back(ColumnDescriptor{ std::move(d1), "a"});

    ColumnDefinition<int> col1{ "value", "value", nullptr };

    // Initializer list can't be moved.
    //const std::vector<ColumnDefinition<int>> cols1 =
    //{
    //    ColumnDefinition<int>{"value", "value", nullptr}
    //};
}
