#include "Awl/Testing/UnitTest.h"

#include <boost/asio.hpp>
#include <boost/asio/redirect_error.hpp>
#include <boost/asio/stream_file.hpp>
#include <boost/asio/use_awaitable.hpp>
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

    class Example
    {
    public:

        Example(const awl::testing::TestContext& context) : context(context) {}

        void runSingleThread()
        {
            log(std::format("Thread {}. runSingleThread() has started.", std::this_thread::get_id()));

            asio::io_context io;

            asio::co_spawn(io, copyFile(), asio::detached);

            io.run();

            log(std::format("Thread {}. runSingleThread() has finished.", std::this_thread::get_id()));
        }

        void runThreadPool()
        {
            log(std::format("Thread {}. runThreadPool() has started.", std::this_thread::get_id()));

            AWL_ATTRIBUTE(size_t, thread_count, std::max(1u, std::thread::hardware_concurrency()));

            asio::io_context io;

            asio::thread_pool pool(thread_count);

            asio::co_spawn(pool, copyFile(), asio::detached);

            io.run();

            pool.join();

            log(std::format("Thread {}. runThreadPool() has finished.", std::this_thread::get_id()));
        }

        void runStrand1()
        {
            log(std::format("Thread {}. runStrand1() has started.", std::this_thread::get_id()));

            AWL_ATTRIBUTE(size_t, thread_count, std::max(1u, std::thread::hardware_concurrency()));

            asio::io_context io;

            asio::thread_pool pool(thread_count);

            asio::strand<asio::thread_pool::executor_type> strand(pool.get_executor());

            asio::co_spawn(strand, copyFile(), asio::detached);

            io.run();

            pool.join();

            log(std::format("Thread {}. runStrand1() has finished.", std::this_thread::get_id()));
        }

        void runStrand2()
        {
            log(std::format("Thread {}. runStrand2() has started.", std::this_thread::get_id()));

            AWL_ATTRIBUTE(size_t, thread_count, std::max(1u, std::thread::hardware_concurrency()));

            asio::io_context io;

            asio::thread_pool pool(thread_count);

            asio::thread_pool pool1(1);

            opExecutor = pool.get_executor();

            asio::co_spawn(pool1, copyFile(), asio::detached);

            io.run();

            pool1.join();

            pool.join();

            log(std::format("Thread {}. runStrand2() has finished.", std::this_thread::get_id()));
        }

        void runCopyPipeline1(asio::any_io_executor exec, bool use_handler, std::function<void()> run)
        {
            AWL_ATTRIBUTE(size_t, reader_buffer_size, 3);

            Channel reader_chan(exec, reader_buffer_size);
            std::optional<Channel> handler_chan;
            Channel* writer_channl;

            if (use_handler)
            {
                AWL_ATTRIBUTE(size_t, handler_buffer_size, 3);

                handler_chan = Channel(exec, handler_buffer_size);
                asio::co_spawn(exec, handle(reader_chan, *handler_chan), boost::asio::detached);
                writer_channl = &(*handler_chan);
            }
            else
            {
                writer_channl = &reader_chan;
            }

            asio::co_spawn(exec, read(reader_chan), boost::asio::detached);
            asio::co_spawn(exec, write(*writer_channl), boost::asio::detached);

            run();
        }

        awaitable<void> runCopyPipeline2(bool use_handler)
        {
            auto exec = co_await asio::this_coro::executor;

            AWL_ATTRIBUTE(size_t, reader_buffer_size, 3);
            AWL_ATTRIBUTE(size_t, handler_buffer_size, 3);

            Channel reader_chan(exec, reader_buffer_size);
            Channel handler_chan(exec, handler_buffer_size);

            std::vector<awaitable<void>> tasks;

            // using namespace boost::asio::experimental::awaitable_operators;
            // co_await(read(reader_chan) && write(*writer_channl));

            tasks.push_back(read(reader_chan));
            tasks.push_back(write(handler_chan));
            tasks.push_back(handle(reader_chan, handler_chan));

            for (auto& t : tasks)
            {
                co_await std::move(t);
            }
        }

    private:

        awaitable<void> copyFile()
        {
            log(std::format("Thread {}. copyFile() has started.", std::this_thread::get_id()));

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

                log(std::format("Thread {}. {} bytes have been read.", std::this_thread::get_id(), read_size));

                const std::size_t written_size = co_await destination.async_write_some(
                    asio::buffer(buffer.data(), read_size), use_awaitable);

                if (written_size != read_size)
                {
                    throw std::runtime_error(std::format("Read {} bytes, but written {} bytes.", read_size, written_size));
                }

                log(std::format("Thread {}. {} bytes have been written.", std::this_thread::get_id(), written_size));

                total_written += written_size;
            }

            log(std::format("Thread {}. Copied {} bytes.", std::this_thread::get_id(), total_written));
        }

        using Chunk = std::shared_ptr<std::vector<char>>;
        using Channel = boost::asio::experimental::channel<void(boost::system::error_code, Chunk)>;

        awaitable<void> read(Channel& reader_chan)
        {
            log(std::format("Thread {}. read() has started.", std::this_thread::get_id()));

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

                log(std::format("Thread {}. {} bytes have been read.", std::this_thread::get_id(), read_size));

                co_await reader_chan.async_send({}, buffer, use_awaitable);
            }

            // Close the channel to signal that no more messages will be sent
            reader_chan.close();
        }

        awaitable<void> write(Channel& reader_chan)
        {
            log(std::format("Thread {}. write() has started.", std::this_thread::get_id()));

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
                    Chunk buffer = co_await reader_chan.async_receive(asio::use_awaitable);

                    log(awl::format() << "Consumed: " << buffer->size() << " bytes.");

                    const std::size_t written_size = co_await destination.async_write_some(
                        asio::buffer(*buffer), use_awaitable);

                    if (written_size != buffer->size())
                    {
                        throw std::runtime_error(std::format("Read {} bytes, but written {} bytes.", buffer->size(), written_size));
                    }

                    log(std::format("Thread {}. {} bytes have been written.", std::this_thread::get_id(), written_size));

                    total_written += written_size;
                }
            }
            catch (const boost::system::system_error& e)
            {
                // Check if the channel was closed gracefully
                if (e.code() == boost::asio::experimental::error::channel_closed)
                    log("Channel closed, exiting consumer");
                else
                    log(awl::format() << "Receive error: " << e.code().message());
            }

            log(std::format("Thread {}. Totally copied {} bytes.", std::this_thread::get_id(), total_written));
        }
            
        awaitable<void> handle(Channel& reader_chan, Channel& writer_chan)
        {
            log(std::format("Thread {}. handle() has started.", std::this_thread::get_id()));

            std::size_t total_handled = 0;

            try
            {
                for (;;)
                {
                    // Receive a message asynchronously from the channel
                    Chunk buffer = co_await reader_chan.async_receive(asio::use_awaitable);

                    log(std::format("Thread {}. {} bytes have been handled.", std::this_thread::get_id(), buffer->size()));

                    total_handled += buffer->size();

                    co_await writer_chan.async_send({}, buffer, use_awaitable);
                }
            }
            catch (const boost::system::system_error& e)
            {
                // Check if the channel was closed gracefully
                if (e.code() == boost::asio::experimental::error::channel_closed)
                    log("Channel closed, exiting handler");
                else
                    log(awl::format() << "Receive error: " << e.code().message());
            }

            writer_chan.close();

            log(std::format("Thread {}. Totally handled {} bytes.", std::this_thread::get_id(), total_handled));
        }

        void log(awl::LogString message)
        {
            context.logger.debug(message);
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

        const awl::testing::TestContext& context;

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
