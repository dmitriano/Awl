#include "Awl/Coro/Channel.h"
#include "Awl/Coro/Job.h"
#include "Awl/Testing/UnitTest.h"
#include "Helpers/TestDispatcher.h"

#include <algorithm>
#include <array>
#include <memory>
#include <string>
#include <thread>
#include <utility>

namespace
{
    template <class T>
    awl::coro::Job send(
        awl::coro::Channel<T>& channel,
        T value,
        bool& sent,
        bool& closed)
    {
        try
        {
            co_await channel.asyncSend(std::move(value));
            sent = true;
        }
        catch (const awl::coro::ChannelClosedException&)
        {
            closed = true;
        }
    }

    template <class T>
    awl::coro::Job receive(
        awl::coro::Channel<T>& channel,
        T& value,
        bool& received,
        bool& closed)
    {
        try
        {
            value = co_await channel.asyncReceive();
            received = true;
        }
        catch (const awl::coro::ChannelClosedException&)
        {
            closed = true;
        }
    }

    awl::coro::Job sendAndRememberThread(
        awl::coro::Channel<int>& channel,
        int value,
        std::thread::id& start_thread_id,
        std::thread::id& resume_thread_id,
        bool& sent)
    {
        start_thread_id = std::this_thread::get_id();
        co_await channel.asyncSend(value);
        resume_thread_id = std::this_thread::get_id();
        sent = true;
    }

    awl::coro::Job receiveAndRememberThread(
        awl::coro::Channel<int>& channel,
        int& value,
        std::thread::id& start_thread_id,
        std::thread::id& resume_thread_id,
        bool& received)
    {
        start_thread_id = std::this_thread::get_id();
        value = co_await channel.asyncReceive();
        resume_thread_id = std::this_thread::get_id();
        received = true;
    }
}

AWL_TEST(CoroChannel_Rendezvous)
{
    AWL_UNUSED_CONTEXT;

    awl::coro::Channel<int> channel;

    int value = 0;
    bool received = false;
    bool receive_closed = false;
    awl::coro::Job receiver = awl::coro::spawn(receive(channel, value, received, receive_closed));

    AWL_ASSERT(!receiver.done());

    bool sent = false;
    bool send_closed = false;
    awl::coro::Job sender = awl::coro::spawn(send(channel, 7, sent, send_closed));

    AWL_ASSERT(sender.done());
    AWL_ASSERT(receiver.done());
    AWL_ASSERT(sent);
    AWL_ASSERT(!send_closed);
    AWL_ASSERT(received);
    AWL_ASSERT(!receive_closed);
    AWL_ASSERT_EQUAL(7, value);
}

AWL_TEST(CoroChannel_Buffered)
{
    AWL_UNUSED_CONTEXT;

    awl::coro::Channel<std::string> channel(2);

    bool first_sent = false;
    bool first_send_closed = false;
    awl::coro::Job first_sender = awl::coro::spawn(send(channel, std::string("first"), first_sent, first_send_closed));

    bool second_sent = false;
    bool second_send_closed = false;
    awl::coro::Job second_sender = awl::coro::spawn(send(channel, std::string("second"), second_sent, second_send_closed));

    AWL_ASSERT(first_sender.done());
    AWL_ASSERT(second_sender.done());
    AWL_ASSERT(first_sent);
    AWL_ASSERT(second_sent);

    std::string first_value;
    bool first_received = false;
    bool first_receive_closed = false;
    awl::coro::Job first_receiver = awl::coro::spawn(receive(channel, first_value, first_received, first_receive_closed));

    std::string second_value;
    bool second_received = false;
    bool second_receive_closed = false;
    awl::coro::Job second_receiver = awl::coro::spawn(receive(channel, second_value, second_received, second_receive_closed));

    AWL_ASSERT(first_receiver.done());
    AWL_ASSERT(second_receiver.done());
    AWL_ASSERT(first_received);
    AWL_ASSERT(second_received);
    AWL_ASSERT_EQUAL(std::string("first"), first_value);
    AWL_ASSERT_EQUAL(std::string("second"), second_value);
}

AWL_TEST(CoroChannel_BackPressure)
{
    AWL_UNUSED_CONTEXT;

    awl::coro::Channel<int> channel(1);

    bool first_sent = false;
    bool first_send_closed = false;
    awl::coro::Job first_sender = awl::coro::spawn(send(channel, 1, first_sent, first_send_closed));

    bool second_sent = false;
    bool second_send_closed = false;
    awl::coro::Job second_sender = awl::coro::spawn(send(channel, 2, second_sent, second_send_closed));

    AWL_ASSERT(first_sender.done());
    AWL_ASSERT(!second_sender.done());
    AWL_ASSERT(first_sent);
    AWL_ASSERT(!second_sent);

    int first_value = 0;
    bool first_received = false;
    bool first_receive_closed = false;
    awl::coro::Job first_receiver = awl::coro::spawn(receive(channel, first_value, first_received, first_receive_closed));

    AWL_ASSERT(first_receiver.done());
    AWL_ASSERT(second_sender.done());
    AWL_ASSERT(second_sent);
    AWL_ASSERT_EQUAL(1, first_value);

    int second_value = 0;
    bool second_received = false;
    bool second_receive_closed = false;
    awl::coro::Job second_receiver = awl::coro::spawn(receive(channel, second_value, second_received, second_receive_closed));

    AWL_ASSERT(second_receiver.done());
    AWL_ASSERT(second_received);
    AWL_ASSERT_EQUAL(2, second_value);
}

AWL_TEST(CoroChannel_CloseWaitingReceiver)
{
    AWL_UNUSED_CONTEXT;

    awl::coro::Channel<int> channel;

    int value = 0;
    bool received = false;
    bool receive_closed = false;
    awl::coro::Job receiver = awl::coro::spawn(receive(channel, value, received, receive_closed));

    AWL_ASSERT(!receiver.done());

    channel.close();

    AWL_ASSERT(receiver.done());
    AWL_ASSERT(!channel.isOpen());
    AWL_ASSERT(!received);
    AWL_ASSERT(receive_closed);
}

AWL_TEST(CoroChannel_CloseKeepsBufferedValues)
{
    AWL_UNUSED_CONTEXT;

    awl::coro::Channel<int> channel(1);

    bool sent = false;
    bool send_closed = false;
    awl::coro::Job sender = awl::coro::spawn(send(channel, 3, sent, send_closed));

    AWL_ASSERT(sender.done());

    channel.close();

    int value = 0;
    bool received = false;
    bool receive_closed = false;
    awl::coro::Job receiver = awl::coro::spawn(receive(channel, value, received, receive_closed));

    AWL_ASSERT(receiver.done());
    AWL_ASSERT(received);
    AWL_ASSERT(!receive_closed);
    AWL_ASSERT_EQUAL(3, value);

    int closed_value = 0;
    bool closed_received = false;
    bool closed_receive_closed = false;
    awl::coro::Job closed_receiver = awl::coro::spawn(receive(channel, closed_value, closed_received, closed_receive_closed));

    AWL_ASSERT(closed_receiver.done());
    AWL_ASSERT(!closed_received);
    AWL_ASSERT(closed_receive_closed);
}

AWL_TEST(CoroChannel_SendToClosedChannel)
{
    AWL_UNUSED_CONTEXT;

    awl::coro::Channel<int> channel;
    channel.close();

    bool sent = false;
    bool send_closed = false;
    awl::coro::Job sender = awl::coro::spawn(send(channel, 5, sent, send_closed));

    AWL_ASSERT(sender.done());
    AWL_ASSERT(!sent);
    AWL_ASSERT(send_closed);
}

AWL_TEST(CoroChannel_SendAndReceiveResumeOnCallingDispatchers)
{
    AWL_UNUSED_CONTEXT;

    awl::coro::Channel<int> channel;
    std::shared_ptr<awl::testing::TestDispatcher> producer_dispatcher =
        std::make_shared<awl::testing::TestDispatcher>();
    std::shared_ptr<awl::testing::TestDispatcher> consumer_dispatcher =
        std::make_shared<awl::testing::TestDispatcher>();

    std::thread::id producer_start_thread;
    std::thread::id producer_resume_thread;
    std::thread::id consumer_start_thread;
    std::thread::id consumer_resume_thread;
    int value = 0;
    bool sent = false;
    bool received = false;

    awl::coro::Job producer = awl::coro::spawn(
        sendAndRememberThread(channel, 17, producer_start_thread, producer_resume_thread, sent),
        producer_dispatcher);

    producer_dispatcher->join();

    AWL_ASSERT(!producer.done());
    AWL_ASSERT(!sent);

    awl::coro::Job consumer = awl::coro::spawn(
        receiveAndRememberThread(channel, value, consumer_start_thread, consumer_resume_thread, received),
        consumer_dispatcher);

    consumer_dispatcher->join();
    producer_dispatcher->join();

    AWL_ASSERT(producer.done());
    AWL_ASSERT(consumer.done());
    AWL_ASSERT(sent);
    AWL_ASSERT(received);
    AWL_ASSERT_EQUAL(17, value);
    AWL_ASSERT(producer_start_thread != std::thread::id());
    AWL_ASSERT(producer_resume_thread != std::thread::id());
    AWL_ASSERT(consumer_start_thread != std::thread::id());
    AWL_ASSERT(consumer_resume_thread != std::thread::id());
    AWL_ASSERT(producer_start_thread == producer_resume_thread);
    AWL_ASSERT(consumer_start_thread == consumer_resume_thread);
    AWL_ASSERT(producer_resume_thread != consumer_resume_thread);
}
