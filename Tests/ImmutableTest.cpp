/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Experimental/Immutable.h"

#include "Awl/Testing/UnitTest.h"
#include "Awl/StringFormat.h"
#include "Awl/IntRange.h"
#include "Awl/Observable.h"

#include <vector>
#include <memory>

namespace
{
    struct A
    {
        int x;
        std::string y;

        void setX(int val)
        {
            x = std::move(val);
        }

        void setY(std::string val)
        {
            y = std::move(val);
        }

        void setXY(int x_val, std::string y_val)
        {
            setX(std::move(x_val));
            setY(std::move(y_val));
        }

        bool operator ==(const A& other) const = default;
    };

    struct UniqueA : A
    {
        UniqueA(int x, std::string y) : 
            A(std::move(x), std::move(y)),
            p(std::make_unique<int>(x))
        {}

        bool operator ==(const UniqueA& other) const
        {
            return *(static_cast<const A*>(this)) == static_cast<const A&>(other);
        }

        std::unique_ptr<int> p;
    };

    class ASignalHandler
    {
    public:

        virtual void onASignal(int val) = 0;

        virtual ~ASignalHandler() = default;
    };

    struct ObserverA : A, awl::Observer<ASignalHandler>
    {
        ObserverA(int x, std::string y) :
            A(std::move(x), std::move(y))
        {
        }

        bool operator ==(const UniqueA& other) const
        {
            return *(static_cast<const A*>(this)) == static_cast<const A&>(other);
        }

        void onASignal(const int val) override
        {
            x = val;
        }
    };

    // We std::unique_ptr with AtomicA, but not immutable.
    struct AtomicA : A
    {
        AtomicA(int x, std::string y) : A(std::move(x), std::move(y)) {}

        bool operator ==(const AtomicA& other) const
        {
            std::lock_guard guard(m);

            return *(static_cast<const A*>(this)) == static_cast<const A&>(other);
        }

        // std::mutex cannot be moved or copied
        mutable std::mutex m;
    };

    template <class A>
    class Test : public awl::testing::Test
    {
    public:

        using awl::testing::Test::Test;

        void ImmutableConstructorAndOperators()
        {
            awl::immutable<A> a1 = makeA();

            // Compiler error:
            // a2->x = 10;

            context.logger.debug(awl::format() << a1->x << " " << a1->y);

            // Copy constructor.
            awl::immutable<A> a2 = a1;

            context.logger.debug(awl::format() << a2->x << " " << a2->y);

            AWL_ASSERT(a2 == a1);

            // Move constructor
            awl::immutable<A> a3 = std::move(a1);

            A mutable_a = a3.release();

            mutable_a.x = 10;

            // Make it immutable again
            awl::immutable<A> a4 = awl::make_immutable<A>(std::move(mutable_a));
        }

        void ImmutableVector()
        {
            AWL_ATTRIBUTE(size_t, insert_count, 1000);

            std::vector<awl::immutable<A>> v;

            const std::string long_string = "A very long string that is not copied, but moved.";

            for (size_t i = 0; i < insert_count; ++i)
            {
                std::ostringstream out;

                out << long_string << " " << i;

                v.push_back(awl::make_immutable<A>(static_cast<int>(i), out.str()));
            }
        }

        void ImmutablePointer()
        {
            // This is possible, but does not make a sense.
            {
                std::unique_ptr<awl::immutable<A>> p = std::make_unique<awl::immutable<A>>(makeA());

                [[maybe_unused]] int x = (*p)->x;
            }

            // To create a smart pointer from awl::immutable we use release method.
            {
                awl::immutable<A> a = makeA();

                std::unique_ptr<A> p = std::make_unique<A>(a.release());

                [[maybe_unused]] int x1 = p->x;

                std::unique_ptr<const A> const_p = std::make_unique<const A>(a.release());

                [[maybe_unused]] int x2 = const_p->x;
            }
        }

    private:

        constexpr awl::immutable<A> makeA()
        {
            return awl::make_immutable<A>(5, "abc");
        }
    };
}

AWL_TEST(ImmutableWith)
{
    AWL_UNUSED_CONTEXT;

    // We will not move a1, so it is const.
    const awl::immutable<A> a1 = awl::make_immutable<A>(5, "abc");

    const awl::immutable<A> expected = awl::make_immutable<A>(10, "def");

    {
        // The first with() copyies a1 because it is value,
        // and the second with moves the result, because it is rvalue.
        awl::immutable<A> a2 = a1.with(&A::setX, 9).with(&A::setY, std::string("def"));

        // We can move a2 explicitly.
        awl::immutable<A> a3 = std::move(a2).with(&A::setX, 10);

        // std::invoke is not constexpr yet, at least in MSVC, but it will be in C++23.
        // static_assert(a3 == awl::make_immutable<A>(10, "def"));

        AWL_ASSERT(a3 == expected);
    }

    AWL_ASSERT(a1.with(&A::setXY, 10, "def") == expected);

    {
        auto func = [](A& a)
            {
                a.x = 10;
                a.y = "def";
            };

        AWL_ASSERT(a1.with(func) == expected);
    }
}

AWL_TEST(ImmutableConstructorAndOperators)
{
    // ImmutableConstructorAndOperators test requires A to be copyable.
    Test<A>{context}.ImmutableConstructorAndOperators();
}

AWL_TEST(ImmutableVector)
{
    Test<A>{context}.ImmutableVector();
    Test<UniqueA>{context}.ImmutableVector();
}

AWL_TEST(ImmutablePointer)
{
    Test<A>{context}.ImmutablePointer();
    Test<UniqueA>{context}.ImmutablePointer();
}

namespace
{
    class ImmutableObserver :
        public awl::testing::Test,
        public awl::Observable<ASignalHandler>
    {
    public:

        using awl::testing::Test::Test;

        void run()
        {
            emitSignal(makeVector());
        }

    private:

        static inline const std::string long_string = "A very long string that is not copied, but moved.";

        std::vector<awl::immutable<ObserverA>> makeVector()
        {
            AWL_ATTRIBUTE(size_t, insert_count, 1000);

            std::vector<awl::immutable<ObserverA>> v;

            for (size_t i = 0; i < insert_count; ++i)
            {
                ObserverA a(static_cast<int>(i), long_string);

                Subscribe(&a);

                // The address of a changes here, but it still handles the signals.
                v.push_back(std::move(a));
            }

            return v;
        }

        // emitSignal accepts a vector of "const" or immutable elements,
        // but the elements can change while handling the signal
        // and the vector itself can be changed.
        // Note that std::vector<const ObserverA> does not compile.
        void emitSignal(std::vector<awl::immutable<ObserverA>> v)
        {
            // Move the elements one more time.
            std::reverse(v.begin(), v.end());

            const int val = 10;

            Notify(&ASignalHandler::onASignal, val);

            for (const awl::immutable<ObserverA>& ia : v)
            {
                // error C3892 : 'ia' : you cannot assign to a variable that is const
                // ia->x = 25;

                AWL_ASSERT_EQUAL(ia->x, val);
                AWL_ASSERT_EQUAL(ia->y, long_string);
            }
        }
    };
}

AWL_TEST_CLASS(ImmutableObserver)

namespace
{
    // We can't return a const string, but we can return immutable string.
    awl::immutable<std::string> makeString()
    {
        std::ostringstream out;

        out << "a temporary string";

        // We can't return it as const std::string&
        return out.str();
    }
}

AWL_TEST(ImmutableString)
{
    awl::immutable<std::string> val = makeString();

    // We use val like std::optional

    [[maybe_unused]] auto symbol = (*val)[0];

    // error C3892: 'val': you cannot assign to a variable that is const
    // (*val)[0] = 'z';

    for (auto symbol : *val)
    {
        static_cast<void>(val);
    }
}

#ifdef AWL_DEBUG_IMMUTABLE

namespace
{
    void func(const awl::testing::TestContext& context, awl::immutable<A> a)
    {
        context.logger.debug(awl::format() << a->x << " " << a->y);
    }
}

AWL_TEST(ImmutableException)
{
    awl::immutable<A> a = awl::make_immutable<A>(5, "abc");

    func(context, std::move(a));

    bool thrown = false;

    try
    {
        int x = a->x;

        context.logger.debug(awl::format() << a->x);
    }
    catch (const std::exception& e)
    {
        thrown = true;

        context.logger.debug(e.what());
    }

    AWL_ASSERT(thrown);
}

#endif

// The code demonstrating why do we need immutable.
namespace
{
    // B becomes non-copyable and non-movable.
    struct B
    {
        B() : p(std::make_unique<int>(5)) {}

        const std::unique_ptr<int> p;
    };

    B f1()
    {
        // RVO, so there is no compiler error.
        return {};
    }

    B f2(bool flag)
    {
        // Different variables -> NRVO impossible.
        B b1;
        B b2;

        if (flag)
        {
            // error C2280: '`anonymous-namespace'::B::B(const `anonymous-namespace'::B &)': attempting to reference a deleted function
            // return b1;
        }

        // The same error.
        // return b2;
    }

    B f3()
    {
        // error C2672: 'std::construct_at': no matching overloaded function found
        // std::vector<B> v;
        // v.push_back(B{});
    }
}
