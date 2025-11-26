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

    class FakeProcessor final : public awl::testing::Test, public VectorProcessor
    {
    public:

        FakeProcessor(const awl::testing::TestContext& context, VectorChannel& input_chan) :
            Test(context),
            m_inputChan(input_chan)
        {}

        awaitable<void> run() override
        {
            print("FakeProcessor that does nothing.");

            co_return;
        }

        VectorChannel& outputChannel() override
        {
            return m_inputChan;
        }

    private:

        VectorChannel& m_inputChan;
    };

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
                    print("VectorChannel closed, exiting handler");
                else
                    print(awl::format() << "Receive error: " << e.code().message());
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

    class Example : public awl::testing::Test
    {
    public:

        using Test::Test;

        void runSingleThread()
        {
            print(std::format("Thread {}. runSingleThread() has started.", std::this_thread::get_id()));

            asio::io_context io;

            asio::co_spawn(io, copyFile(), asio::detached);

            io.run();

            print(std::format("Thread {}. runSingleThread() has finished.", std::this_thread::get_id()));
        }

        void runThreadPool()
        {
            print(std::format("Thread {}. runThreadPool() has started.", std::this_thread::get_id()));

            AWL_ATTRIBUTE(size_t, thread_count, std::max(1u, std::thread::hardware_concurrency()));

            asio::io_context io;

            asio::thread_pool pool(thread_count);

            asio::co_spawn(pool, copyFile(), asio::detached);

            io.run();

            pool.join();

            print(std::format("Thread {}. runThreadPool() has finished.", std::this_thread::get_id()));
        }

        void runStrand1()
        {
            print(std::format("Thread {}. runStrand1() has started.", std::this_thread::get_id()));

            AWL_ATTRIBUTE(size_t, thread_count, std::max(1u, std::thread::hardware_concurrency()));

            asio::io_context io;

            asio::thread_pool pool(thread_count);

            asio::strand<asio::thread_pool::executor_type> strand(pool.get_executor());

            asio::co_spawn(strand, copyFile(), asio::detached);

            io.run();

            pool.join();

            print(std::format("Thread {}. runStrand1() has finished.", std::this_thread::get_id()));
        }

        void runStrand2()
        {
            print(std::format("Thread {}. runStrand2() has started.", std::this_thread::get_id()));

            AWL_ATTRIBUTE(size_t, thread_count, std::max(1u, std::thread::hardware_concurrency()));

            asio::io_context io;

            asio::thread_pool pool(thread_count);

            asio::thread_pool pool1(1);

            opExecutor = pool.get_executor();

            asio::co_spawn(pool1, copyFile(), asio::detached);

            io.run();

            pool1.join();

            pool.join();

            print(std::format("Thread {}. runStrand2() has finished.", std::this_thread::get_id()));
        }

        std::shared_ptr<VectorProcessor> makeHandler(asio::any_io_executor exec, bool use_handler, VectorChannel& reader_chan) const
        {
            std::shared_ptr<VectorProcessor> handler;

            if (use_handler)
            {
                AWL_ATTRIBUTE(size_t, handler_buffer_size, 3);

                handler = std::make_shared<PrintProcessor>(context, reader_chan, VectorChannel(exec, handler_buffer_size));
            }
            else
            {
                handler = std::make_shared<FakeProcessor>(context, reader_chan);
            }

            return handler;
        }

        void runCopyPipeline1(asio::any_io_executor exec, bool use_handler, std::function<void()> run)
        {
            AWL_ATTRIBUTE(size_t, reader_buffer_size, 3);

            VectorChannel reader_chan(exec, reader_buffer_size);

            std::shared_ptr<VectorProcessor> handler = makeHandler(exec, use_handler, reader_chan);

            asio::co_spawn(exec, read(reader_chan), boost::asio::detached);
            asio::co_spawn(exec, handler->run(), boost::asio::detached);
            asio::co_spawn(exec, write(handler->outputChannel()), boost::asio::detached);

            run();
        }

        awaitable<void> runCopyPipeline2(bool use_handler)
        {
            auto exec = co_await asio::this_coro::executor;

            AWL_ATTRIBUTE(size_t, reader_buffer_size, 3);
            AWL_ATTRIBUTE(size_t, handler_buffer_size, 3);

            VectorChannel reader_chan(exec, reader_buffer_size);
            VectorChannel handler_chan(exec, handler_buffer_size);

            std::vector<awaitable<void>> tasks;

            // using namespace boost::asio::experimental::awaitable_operators;
            // co_await(read(reader_chan) && write(*writer_channl));

            tasks.push_back(read(reader_chan));
            tasks.push_back(write(handler_chan));
            // tasks.push_back(handle(reader_chan, handler_chan));

            for (auto& t : tasks)
            {
                co_await std::move(t);
            }
        }

        // Probably this version of runCopyPipeline is correct.
        awaitable<void> runCopyPipeline3(bool use_handler)
        {
            auto exec = co_await asio::this_coro::executor;

            AWL_ATTRIBUTE(size_t, reader_buffer_size, 3);

            VectorChannel reader_chan(exec, reader_buffer_size);

            std::shared_ptr<VectorProcessor> handler = makeHandler(exec, use_handler, reader_chan);

            using namespace boost::asio::experimental::awaitable_operators;

            try
            {
                // When one the of the tasks throws an exception the others should be cancelled.
                co_await(asio::co_spawn(exec, read(reader_chan), asio::use_awaitable) &&
                    asio::co_spawn(exec, handler->run(), asio::use_awaitable) &&
                    asio::co_spawn(exec, write(handler->outputChannel()), asio::use_awaitable));
            }
            catch (const std::exception& e)
            {
                print(std::format("Pipeline Exception: {}", e.what()));
            }
        }

        awaitable<void> runCopyPipeline4(bool use_handler)
        {
            auto exec = co_await asio::this_coro::executor;

            AWL_ATTRIBUTE(size_t, reader_buffer_size, 3);

            VectorChannel reader_chan(exec, reader_buffer_size);

            std::shared_ptr<VectorProcessor> handler = makeHandler(exec, use_handler, reader_chan);

            std::vector<awaitable<void>> tasks;

            tasks.push_back(asio::co_spawn(exec, read(reader_chan), asio::use_awaitable));
            tasks.push_back(asio::co_spawn(exec, handler->run(), asio::use_awaitable));
            tasks.push_back(asio::co_spawn(exec, write(handler->outputChannel()), asio::use_awaitable));

            for (auto& t : tasks)
            {
                co_await std::move(t);
            }
        }

    private:

        awaitable<void> copyFile()
        {
            print(std::format("Thread {}. copyFile() has started.", std::this_thread::get_id()));

            asio::any_io_executor exec = opExecutor ? *opExecutor : co_await asio::this_coro::executor;

            asio::stream_file source(exec);
            source.open(sourcePath(), asio::stream_file::read_only);

            asio::stream_file destination(exec);
            destination.open(destinationPath(),
                asio::stream_file::create | asio::stream_file::write_only | asio::stream_file::truncate);

            std::vector<uint8_t> buffer(chunkSize);

            std::size_t total_written = 0;

            for (;;)
            {
                boost::system::error_code ec;
                const std::size_t read_size = co_await source.async_read_some(asio::buffer(buffer),
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

                const std::size_t written_size = co_await destination.async_write_some(
                    asio::buffer(buffer.data(), read_size), use_awaitable);

                if (written_size != read_size)
                {
                    throw std::runtime_error(std::format("Read {} bytes, but written {} bytes.", read_size, written_size));
                }

                print(std::format("Thread {}. {} bytes have been written.", std::this_thread::get_id(), written_size));

                total_written += written_size;
            }

            print(std::format("Thread {}. Copied {} bytes.", std::this_thread::get_id(), total_written));
        }

        awaitable<void> read(VectorChannel& reader_chan)
        {
            print(std::format("Thread {}. read() has started.", std::this_thread::get_id()));

            asio::any_io_executor exec = opExecutor ? *opExecutor : co_await asio::this_coro::executor;

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

                co_await reader_chan.async_send({}, buffer, use_awaitable);
            }

            // Close the channel to signal that no more messages will be sent
            reader_chan.close();
        }

        awaitable<void> write(VectorChannel& reader_chan)
        {
            print(std::format("Thread {}. write() has started.", std::this_thread::get_id()));

            asio::any_io_executor exec = opExecutor ? *opExecutor : co_await asio::this_coro::executor;

            asio::stream_file destination(exec);
            destination.open(destinationPath(),
                asio::stream_file::create | asio::stream_file::write_only | asio::stream_file::truncate);

            std::size_t total_written = 0;

            try
            {
                for (;;)
                {
                    // Receive a message asynchronously from the channel
                    VectorChunk buffer = co_await reader_chan.async_receive(asio::use_awaitable);

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
                    print("VectorChannel closed, exiting consumer");
                else
                    print(awl::format() << "Receive error: " << e.code().message());
            }

            print(std::format("Thread {}. Totally copied {} bytes.", std::this_thread::get_id(), total_written));
        }
            
        template <class Rep, class Period>
        awaitable<void> sleep(std::chrono::duration<Rep, Period> d)
        {
            // Wait for 500 ms before sending the next message
            co_await asio::steady_timer(co_await asio::this_coro::executor, d).async_wait(use_awaitable);
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

        std::optional<asio::any_io_executor> opExecutor;
    };
}

#ifdef CopyFile
#undef CopyFile
#endif

// head -c 163840 AwlTest.pdb > input.dat

AWL_EXAMPLE(CopyFileSingleThread)
{
    Example example{ context };

    example.runSingleThread();
}

AWL_EXAMPLE(CopyFileThreadPool)
{
    Example example{ context };

    example.runThreadPool();
}

AWL_EXAMPLE(CopyFileStrand1)
{
    Example example{ context };

    example.runStrand1();
}

AWL_EXAMPLE(CopyFileStrand2)
{
    Example example{ context };

    example.runStrand2();
}

AWL_EXAMPLE(CopyFileWithChannel1)
{
    Example example{ context };

    AWL_FLAG(on_pool);
    AWL_FLAG(use_handler);

    if (on_pool)
    {
        AWL_ATTRIBUTE(size_t, thread_count, std::max(1u, std::thread::hardware_concurrency()));

        asio::thread_pool pool(thread_count);

        example.runCopyPipeline1(pool.get_executor(), use_handler, [&pool]() { pool.join(); });
    }
    else
    {
        asio::io_context io;

        example.runCopyPipeline1(io.get_executor(), use_handler, [&io]() { io.run(); });
    }
}

AWL_EXAMPLE(CopyFileWithChannel2)
{
    Example example{ context };

    AWL_FLAG(on_pool);
    AWL_FLAG(use_handler);

    if (on_pool)
    {
        AWL_ATTRIBUTE(size_t, thread_count, std::max(1u, std::thread::hardware_concurrency()));

        asio::thread_pool pool(thread_count);

        asio::co_spawn(pool, example.runCopyPipeline2(use_handler), asio::detached);

        pool.join();

    }
    else
    {
        asio::io_context io;

        asio::co_spawn(io, example.runCopyPipeline2(use_handler), asio::detached);

        io.run();
    }
}

// This test handles the exceptions correctly.
// --filter CopyFileWithChannel3.* --output all --use_handler --on_pool --src input.dat --dst input.dat.copy
AWL_EXAMPLE(CopyFileWithChannel3)
{
    Example example{ context };

    AWL_FLAG(on_pool);
    AWL_FLAG(use_handler);

    if (on_pool)
    {
        AWL_ATTRIBUTE(size_t, thread_count, std::max(1u, std::thread::hardware_concurrency()));

        asio::thread_pool pool(thread_count);

        asio::co_spawn(pool, example.runCopyPipeline3(use_handler), asio::detached);

        pool.join();

    }
    else
    {
        asio::io_context io;

        asio::co_spawn(io, example.runCopyPipeline3(use_handler), asio::detached);

        io.run();
    }
}

AWL_EXAMPLE(CopyFileWithChannel4)
{
    Example example{ context };

    AWL_FLAG(on_pool);
    AWL_FLAG(use_handler);

    if (on_pool)
    {
        AWL_ATTRIBUTE(size_t, thread_count, std::max(1u, std::thread::hardware_concurrency()));

        asio::thread_pool pool(thread_count);

        asio::co_spawn(pool, example.runCopyPipeline4(use_handler), asio::detached);

        pool.join();

    }
    else
    {
        asio::io_context io;

        asio::co_spawn(io, example.runCopyPipeline4(use_handler), asio::detached);

        io.run();
    }
}
