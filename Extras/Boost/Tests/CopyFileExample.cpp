#include "Awl/Testing/UnitTest.h"

#include <boost/asio.hpp>
#include <boost/asio/redirect_error.hpp>
#include <boost/asio/stream_file.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/system/system_error.hpp>

#include <cstdint>
#include <vector>
#include <format>
#include <thread>

namespace asio = boost::asio;
using asio::awaitable;
using asio::use_awaitable;

namespace
{
    constexpr std::size_t chunkSize = 64 * 1024;

    struct Example
    {
        const awl::testing::TestContext& context;

        void run()
        {
            context.logger.debug(std::format("Thread {}. run() has started.", std::this_thread::get_id()));

            AWL_ATTRIBUTE(std::string, input, "input.dat");
            AWL_ATTRIBUTE(std::string, output, "output.dat");

            asio::io_context exec;

            asio::co_spawn(exec, copyFile(input, output), asio::detached);

            exec.run();

            context.logger.debug(std::format("Thread {}. run() has finished.", std::this_thread::get_id()));
        }

        awaitable<void> copyFile(const std::string& source_path, const std::string& destination_path)
        {
            context.logger.debug(std::format("Thread {}. copyFile() has started.", std::this_thread::get_id()));

            auto exec = co_await asio::this_coro::executor;

            asio::stream_file source(exec);
            source.open(source_path, asio::stream_file::read_only);

            asio::stream_file destination(exec);
            destination.open(destination_path,
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

                context.logger.debug(std::format("Thread {}. {} bytes have been read.", std::this_thread::get_id(), read_size));

                const std::size_t written_size = co_await destination.async_write_some(
                    asio::buffer(buffer.data(), read_size), use_awaitable);

                if (written_size != read_size)
                {
                    throw std::runtime_error(std::format("Read {} bytes, but written {} bytes.", read_size, written_size));
                }

                context.logger.debug(std::format("Thread {}. {} bytes have been written.", std::this_thread::get_id(), written_size));

                total_written += written_size;
            }

            context.logger.debug(std::format("Thread {}. Copied {} bytes.", std::this_thread::get_id(), total_written));
        }
    };
}

#ifdef CopyFile
#undef CopyFile
#endif

// head -c 163840 AwlTest.pdb > input.dat
// 
AWL_EXAMPLE(CopyFile)
{
    Example example{ context };

    example.run();
}
