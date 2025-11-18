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

        void runCopyPipeline()
        {
            asio::io_context io;

            Channel reader_chan(io.get_executor(), 3);

            asio::co_spawn(io, read(reader_chan), boost::asio::detached);
            asio::co_spawn(io, write(reader_chan), boost::asio::detached);

            io.run();
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

            log(std::format("Thread {}. Copied {} bytes.", std::this_thread::get_id(), total_written));
        }
            
        void log(awl::LogString message)
        {
            context.logger.debug(message);
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

AWL_EXAMPLE(CopyFileWithChannel)
{
    Example example{ context };

    example.runCopyPipeline();
}
