/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Experimental/SignalOnObservable.h"
#include "Awl/Observer.h"
#include "Awl/Testing/UnitTest.h"

#include <utility>
#include <vector>

namespace
{
    struct IValueSlot
    {
        virtual void operator()(int value) = 0;
    };

    class ValueSignal : public awl::SignalOnObservable<IValueSlot>
    {
    public:

        void emitValue(int value)
        {
            emit(value);
        }
    };

    class CountingValueSlot : public awl::Observer<IValueSlot>
    {
    public:

        CountingValueSlot() = default;

        CountingValueSlot(int* p_count, int* p_last_value)
            : pCount(p_count), pLastValue(p_last_value)
        {
        }

        void operator()(int value) override
        {
            ++(*pCount);
            *pLastValue = value;
        }

    private:

        int* pCount = nullptr;
        int* pLastValue = nullptr;
    };

    class TrackingValueSlot : public awl::Observer<IValueSlot>
    {
    public:

        TrackingValueSlot() = default;

        TrackingValueSlot(int tag, std::vector<int>* p_events)
            : m_tag(tag), pEvents(p_events)
        {
        }

        void setTag(int tag)
        {
            m_tag = tag;
        }

        void operator()(int value) override
        {
            pEvents->push_back(m_tag * 1000 + value);
        }

    private:

        int m_tag = 0;
        std::vector<int>* pEvents = nullptr;
    };

    struct IPredicateSlot
    {
        virtual bool operator()(int value) = 0;
    };

    class PredicateSignal : public awl::SignalOnObservable<IPredicateSlot>
    {
    public:

        bool emitValueWhileTrue(int value)
        {
            return emitWhileTrue(value);
        }
    };

    class PredicateSlot : public awl::Observer<IPredicateSlot>
    {
    public:

        PredicateSlot(bool result, int* p_count, int* p_last_value)
            : m_result(result), pCount(p_count), pLastValue(p_last_value)
        {
        }

        bool operator()(int value) override
        {
            ++(*pCount);
            *pLastValue = value;
            return m_result;
        }

    private:

        bool m_result = false;
        int* pCount = nullptr;
        int* pLastValue = nullptr;
    };
}

AWL_TEST(SignalOnObservable_Events)
{
    AWL_UNUSED_CONTEXT;

    ValueSignal signal;

    AWL_ASSERT(signal.empty());
    AWL_ASSERT_EQUAL(0U, signal.size());

    int count1 = 0;
    int count2 = 0;
    int count3 = 0;
    int last1 = 0;
    int last2 = 0;
    int last3 = 0;

    {
        CountingValueSlot slot1(&count1, &last1);
        CountingValueSlot slot2(&count2, &last2);
        CountingValueSlot slot3(&count3, &last3);

        signal.subscribe(&slot1);
        signal.subscribe(&slot2);
        signal.subscribe(&slot3);

        AWL_ASSERT_EQUAL(3u, signal.size());

        signal.emitValue(10);

        AWL_ASSERT_EQUAL(1, count1);
        AWL_ASSERT_EQUAL(1, count2);
        AWL_ASSERT_EQUAL(1, count3);
        AWL_ASSERT_EQUAL(10, last1);
        AWL_ASSERT_EQUAL(10, last2);
        AWL_ASSERT_EQUAL(10, last3);

        slot1.unsubscribeSelf();
        signal.unsubscribe(&slot2);

        AWL_ASSERT_EQUAL(1u, signal.size());

        signal.emitValue(20);

        AWL_ASSERT_EQUAL(1, count1);
        AWL_ASSERT_EQUAL(1, count2);
        AWL_ASSERT_EQUAL(2, count3);
        AWL_ASSERT_EQUAL(20, last3);
    }

    AWL_ASSERT(signal.empty());
    AWL_ASSERT_EQUAL(0U, signal.size());
}

AWL_TEST(SignalOnObservable_Move)
{
    AWL_UNUSED_CONTEXT;

    ValueSignal signal1;

    int count1 = 0;
    int count2 = 0;
    int last1 = 0;
    int last2 = 0;

    CountingValueSlot slot1(&count1, &last1);
    CountingValueSlot slot2(&count2, &last2);

    signal1.subscribe(&slot1);
    signal1.subscribe(&slot2);

    ValueSignal signal2 = std::move(signal1);
    signal2.emitValue(5);

    AWL_ASSERT_EQUAL(1, count1);
    AWL_ASSERT_EQUAL(1, count2);
    AWL_ASSERT_EQUAL(5, last1);
    AWL_ASSERT_EQUAL(5, last2);

    ValueSignal signal3;
    signal3 = std::move(signal2);
    signal3.emitValue(7);

    AWL_ASSERT_EQUAL(2, count1);
    AWL_ASSERT_EQUAL(2, count2);
    AWL_ASSERT_EQUAL(7, last1);
    AWL_ASSERT_EQUAL(7, last2);
}

AWL_TEST(SignalOnObservable_ObserverMove)
{
    AWL_UNUSED_CONTEXT;

    ValueSignal signal;
    std::vector<int> events;

    TrackingValueSlot slot1(1, &events);
    TrackingValueSlot slot2(2, &events);

    signal.subscribe(&slot1);
    signal.subscribe(&slot2);

    TrackingValueSlot slot1_copy = std::move(slot1);

    TrackingValueSlot slot2_copy;
    slot2_copy = std::move(slot2);

    slot1_copy.setTag(10);
    slot2_copy.setTag(20);

    signal.emitValue(5);

    AWL_ASSERTM_FALSE(slot1.isSubscribed(), _T("Slot #1 is still subscribed."));
    AWL_ASSERTM_FALSE(slot2.isSubscribed(), _T("Slot #2 is still subscribed."));
    AWL_ASSERTM_TRUE(slot1_copy.isSubscribed(), _T("Moved slot #1 is not subscribed."));
    AWL_ASSERTM_TRUE(slot2_copy.isSubscribed(), _T("Moved slot #2 is not subscribed."));

    AWL_ASSERT_EQUAL(2u, events.size());
    AWL_ASSERT_EQUAL(10005, events[0]);
    AWL_ASSERT_EQUAL(20005, events[1]);
}

AWL_TEST(SignalOnObservable_EmitWhileTrue_StopsOnFalse)
{
    AWL_UNUSED_CONTEXT;

    PredicateSignal signal;

    int count1 = 0;
    int count2 = 0;
    int count3 = 0;
    int last1 = 0;
    int last2 = 0;
    int last3 = 0;

    PredicateSlot slot1(true, &count1, &last1);
    PredicateSlot slot2(false, &count2, &last2);
    PredicateSlot slot3(true, &count3, &last3);

    signal.subscribe(&slot1);
    signal.subscribe(&slot2);
    signal.subscribe(&slot3);

    const bool result = signal.emitValueWhileTrue(42);

    AWL_ASSERTM_FALSE(result, _T("emitWhileTrue should stop on first false."));
    AWL_ASSERT_EQUAL(1, count1);
    AWL_ASSERT_EQUAL(1, count2);
    AWL_ASSERT_EQUAL(0, count3);
    AWL_ASSERT_EQUAL(42, last1);
    AWL_ASSERT_EQUAL(42, last2);
    AWL_ASSERT_EQUAL(0, last3);
}

AWL_TEST(SignalOnObservable_EmitWhileTrue_AllTrue)
{
    AWL_UNUSED_CONTEXT;

    PredicateSignal signal;

    int count1 = 0;
    int count2 = 0;
    int last1 = 0;
    int last2 = 0;

    PredicateSlot slot1(true, &count1, &last1);
    PredicateSlot slot2(true, &count2, &last2);

    signal.subscribe(&slot1);
    signal.subscribe(&slot2);

    const bool result = signal.emitValueWhileTrue(7);

    AWL_ASSERTM_TRUE(result, _T("emitWhileTrue should return true when all slots return true."));
    AWL_ASSERT_EQUAL(1, count1);
    AWL_ASSERT_EQUAL(1, count2);
    AWL_ASSERT_EQUAL(7, last1);
    AWL_ASSERT_EQUAL(7, last2);
}

#ifdef _MSC_VER

namespace
{
    struct SlotA
    {
        virtual void operator()(int value) = 0;
    };

    struct SlotB
    {
        virtual void operator()(int value) = 0;
    };

    class SignalA : public awl::SignalOnObservable<SlotA>
    {
    public:

        void emitValue(int value)
        {
            emit(value);
        }
    };

    class SignalB : public awl::SignalOnObservable<SlotB>
    {
    public:

        void emitValue(int value)
        {
            emit(value);
        }
    };

    class MultiObserver :
        public awl::Observer<SlotA>,
        public awl::Observer<SlotB>
    {
    public:

        void SlotA::operator()(int value) override
        {
            aValue = value;
        }

        void SlotB::operator()(int value) override
        {
            bValue = value;
        }

        int aValue = 0;
        int bValue = 0;
    };
}

AWL_TEST(SignalOnObservable_MultiObserver_IndependentValues)
{
    AWL_UNUSED_CONTEXT;

    SignalA signalA;
    SignalB signalB;
    MultiObserver observer;

    signalA.subscribe(static_cast<awl::Observer<SlotA>*>(&observer));
    signalB.subscribe(static_cast<awl::Observer<SlotB>*>(&observer));

    AWL_ASSERT_EQUAL(0, observer.aValue);
    AWL_ASSERT_EQUAL(0, observer.bValue);

    signalA.emitValue(11);
    AWL_ASSERT_EQUAL(11, observer.aValue);
    AWL_ASSERT_EQUAL(0, observer.bValue);

    signalB.emitValue(22);
    AWL_ASSERT_EQUAL(11, observer.aValue);
    AWL_ASSERT_EQUAL(22, observer.bValue);

    signalA.emitValue(-3);
    AWL_ASSERT_EQUAL(-3, observer.aValue);
    AWL_ASSERT_EQUAL(22, observer.bValue);
}

AWL_TEST(SignalOnObservable_MultiObserver_UnsubscribeA_DoesNotAffectB)
{
    AWL_UNUSED_CONTEXT;

    SignalA signalA;
    SignalB signalB;
    MultiObserver observer;

    signalA.subscribe(static_cast<awl::Observer<SlotA>*>(&observer));
    signalB.subscribe(static_cast<awl::Observer<SlotB>*>(&observer));

    signalA.unsubscribe(static_cast<awl::Observer<SlotA>*>(&observer));

    signalA.emitValue(100);
    signalB.emitValue(200);

    AWL_ASSERT_EQUAL(0, observer.aValue);
    AWL_ASSERT_EQUAL(200, observer.bValue);
}

#endif
