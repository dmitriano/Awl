#include "Awl/Testing/UnitTest.h"

#include <boost/asio.hpp>
#include <boost/asio/redirect_error.hpp>
#include <boost/asio/stream_file.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/experimental/awaitable_operators.hpp>
#include <boost/system/system_error.hpp>
#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/experimental/channel.hpp>

#include <cstdint>
#include <vector>
#include <format>
#include <thread>
#include <optional>

namespace asio = boost::asio;
using asio::awaitable;
using asio::use_awaitable;
using namespace std::chrono_literals;

namespace
{
    constexpr std::size_t chunkSize = 64 * 1024;

    using VectorChunk = std::shared_ptr<std::vector<char>>;

    template <class Chunk>
    using Channel = boost::asio::experimental::channel<void(boost::system::error_code, Chunk)>;

    // It also can be named DataProducer, because Producer is probably
    // something like a running coroutine that outputs something to a channel.
    template <class Chunk>
    class DataProcessor
    {
    public:

        virtual awaitable<void> run() = 0;

        virtual Channel<Chunk>& outputChannel() = 0;

        // There can be also a method like this
        // virtual void setProgressCallback(std::function<void(Status)) func);
    };

    using VectorChannel = Channel<VectorChunk>;
    using VectorProcessor = DataProcessor<VectorChunk>;

    class PrintProcessor final : public awl::testing::Test, public VectorProcessor
    {
    public:

        PrintProcessor(const awl::testing::TestContext& context, VectorChannel& input_chan, VectorChannel output_chan) :
            Test(context),
            m_inputChan(input_chan),
            m_outputChan(std::move(output_chan))
        {}

        awaitable<void> run() override
        {
            print(std::format("Thread {}. run() has started.", std::this_thread::get_id()));

            std::size_t total_handled = 0;

            try
            {
                for (;;)
                {
                    // Receive a message asynchronously from the channel
                    VectorChunk buffer = co_await m_inputChan.async_receive(asio::use_awaitable);

                    print(std::format("Thread {}. {} bytes have been handled.", std::this_thread::get_id(), buffer->size()));

                    total_handled += buffer->size();

                    co_await m_outputChan.async_send({}, buffer, use_awaitable);
                }
            }
            catch (const boost::system::system_error& e)
            {
                // Check if the channel was closed gracefully
                if (e.code() == boost::asio::experimental::error::channel_closed)
                {
                    print(std::format("Thread {}. VectorChannel closed, exiting handler.", std::this_thread::get_id()));
                }
                else
                {
                    print(awl::format() << "Processing error: " << e.code().message());

                    throw;
                }
            }

            m_outputChan.close();

            print(std::format("Thread {}. Totally handled {} bytes.", std::this_thread::get_id(), total_handled));
        }

        VectorChannel& outputChannel() override
        {
            return m_outputChan;
        }

    private:

        VectorChannel& m_inputChan;
        VectorChannel m_outputChan;
    };

    // This is our session. It is not aware of DataProcessor.
    class FileCopier : public awl::testing::Test
    {
    public:

        FileCopier(const awl::testing::TestContext& context, VectorChannel& reader_channel, VectorChannel& writer_channel) :
            Test(context),
            readerChannel(reader_channel),
            writerChannel(writer_channel)
        {}

        awaitable<void> run()
        {
            auto exec = co_await asio::this_coro::executor;

            AWL_ATTRIBUTE(size_t, reader_buffer_size, 3);

            using namespace boost::asio::experimental::awaitable_operators;

            co_await(asio::co_spawn(exec, read(), asio::use_awaitable) &&
                asio::co_spawn(exec, write(), asio::use_awaitable));
        }

    private:

        // The channel where read() method writes to.
        VectorChannel& readerChannel;

        // The channel where write() method reads from.
        VectorChannel& writerChannel;

        awaitable<void> read()
        {
            print(std::format("Thread {}. read() has started.", std::this_thread::get_id()));

            asio::any_io_executor exec = co_await asio::this_coro::executor;

            asio::stream_file source(exec);
            source.open(sourcePath(), asio::stream_file::read_only);

            auto buffer = std::make_shared<std::vector<char>>(chunkSize);

            for (;;)
            {
                boost::system::error_code ec;
                const std::size_t read_size = co_await source.async_read_some(asio::buffer(*buffer),
                    asio::redirect_error(use_awaitable, ec));

                if (ec == asio::error::eof)
                {
                    break;
                }

                if (ec)
                {
                    throw boost::system::system_error(ec);
                }

                print(std::format("Thread {}. {} bytes have been read.", std::this_thread::get_id(), read_size));

                co_await readerChannel.async_send({}, buffer, use_awaitable);
            }

            // Close the channel to signal that no more messages will be sent
            readerChannel.close();
        }

        awaitable<void> write()
        {
            print(std::format("Thread {}. write() has started.", std::this_thread::get_id()));

            asio::any_io_executor exec = co_await asio::this_coro::executor;

            asio::stream_file destination(exec);
            destination.open(destinationPath(),
                asio::stream_file::create | asio::stream_file::write_only | asio::stream_file::truncate);

            std::size_t total_written = 0;

            try
            {
                for (;;)
                {
                    // Receive a message asynchronously from the channel
                    VectorChunk buffer = co_await writerChannel.async_receive(asio::use_awaitable);

                    print(std::format("Thread{}. Consumed: {} bytes.", std::this_thread::get_id(), buffer->size()));

                    const std::size_t written_size = co_await destination.async_write_some(
                        asio::buffer(*buffer), use_awaitable);

                    if (written_size != buffer->size())
                    {
                        throw std::runtime_error(std::format("Read {} bytes, but written {} bytes.", buffer->size(), written_size));
                    }

                    print(std::format("Thread {}. {} bytes have been written.", std::this_thread::get_id(), written_size));

                    total_written += written_size;
                }
            }
            catch (const boost::system::system_error& e)
            {
                // Check if the channel was closed gracefully
                if (e.code() == boost::asio::experimental::error::channel_closed)
                {
                    print(std::format("Thread {}. VectorChannel closed, exiting consumer.", std::this_thread::get_id()));
                }
                else
                {
                    print(awl::format() << "Receive error: " << e.code().message());

                    throw;
                }
            }

            print(std::format("Thread {}. Totally copied {} bytes.", std::this_thread::get_id(), total_written));
        }

        std::string sourcePath() const
        {
            AWL_ATTRIBUTE(std::string, src, "input.dat");

            return src;
        }

        std::string destinationPath() const
        {
            AWL_ATTRIBUTE(std::string, dst, "output.dat");

            return dst;
        }
    };

    class Example : public awl::testing::Test
    {
    public:

        using Test::Test;

        awaitable<void> run(bool use_handler)
        {
            auto exec = co_await asio::this_coro::executor;

            AWL_ATTRIBUTE(size_t, reader_buffer_size, 3);

            VectorChannel reader_channel(exec, reader_buffer_size);

            std::shared_ptr<VectorProcessor> handler;

            if (use_handler)
            {
                AWL_ATTRIBUTE(size_t, handler_buffer_size, 3);

                handler = std::make_shared<PrintProcessor>(context, reader_channel, VectorChannel(exec, handler_buffer_size));
            }

            try
            {
                co_await internalRun(reader_channel, handler);
            }
            catch (const std::exception& e)
            {
                print(std::format("Pipeline failed with an exception: {}", e.what()));
            }
        }

    private:

        awaitable<void> internalRun(VectorChannel& reader_channel, const std::shared_ptr<VectorProcessor>& handler)
        {
            if (handler)
            {
                auto exec = co_await asio::this_coro::executor;

                FileCopier copier(context, reader_channel, handler->outputChannel());

                using namespace boost::asio::experimental::awaitable_operators;

                // When one the of the tasks throws an exception the others should be cancelled.
                co_await(asio::co_spawn(exec, copier.run(), asio::use_awaitable) &&
                    asio::co_spawn(exec, handler->run(), asio::use_awaitable));
            }
            else
            {
                FileCopier copier(context, reader_channel, reader_channel);

                co_await copier.run();
            }
        }
    };
}

// This test handles the exceptions correctly.
// --filter CopyFileWithHandler.* --output all --use_handler --on_pool --src input.dat --dst output.dat
AWL_EXAMPLE(CopyFileWithHandler)
{
    Example example{ context };

    AWL_FLAG(on_pool);
    AWL_FLAG(use_handler);

    AWL_ATTRIBUTE(size_t, reader_buffer_size, 3);

    if (on_pool)
    {
        AWL_ATTRIBUTE(size_t, thread_count, std::max(1u, std::thread::hardware_concurrency()));

        asio::thread_pool pool(thread_count);

        asio::co_spawn(pool, example.run(use_handler), asio::detached);

        pool.join();
    }
    else
    {
        asio::io_context io;

        asio::co_spawn(io, example.run(use_handler), asio::detached);

        io.run();
    }
}
