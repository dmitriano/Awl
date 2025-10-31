#include "Awl/Testing/UnitTest.h"

#include <boost/asio.hpp>
#include <boost/asio/stream_file.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/redirect_error.hpp>

#include <array>
#include <filesystem>
#include <iostream>
#include <string>

namespace asio = boost::asio;
using asio::awaitable;
using asio::use_awaitable;

namespace
{
    awaitable<void> write_file(const std::filesystem::path& file_path, std::string data)
    {
        auto ex = co_await asio::this_coro::executor;

        asio::stream_file file(ex);
        file.open(file_path, asio::stream_file::create | asio::stream_file::write_only | asio::stream_file::truncate);

        std::size_t total_written = 0;

        while (total_written < data.size())
        {
            total_written += co_await file.async_write_some(
                asio::buffer(data.data() + total_written, data.size() - total_written), use_awaitable);
        }
    }

    awaitable<void> copy_file_in_chunks(const std::filesystem::path& source_path,
        const std::filesystem::path& destination_path)
    {
        auto ex = co_await asio::this_coro::executor;

        asio::stream_file source(ex);
        source.open(source_path, asio::stream_file::read_only);

        asio::stream_file destination(ex);
        destination.open(destination_path,
            asio::stream_file::create | asio::stream_file::write_only | asio::stream_file::truncate);

        std::array<char, 128> buffer{};

        for (;;)
        {
            boost::system::error_code ec;
            const std::size_t read = co_await source.async_read_some(asio::buffer(buffer),
                asio::redirect_error(use_awaitable, ec));

            if (ec == asio::error::eof)
            {
                break;
            }

            if (ec)
            {
                throw boost::system::system_error(ec);
            }

            std::size_t total_written = 0;

            while (total_written < read)
            {
                total_written += co_await destination.async_write_some(
                    asio::buffer(buffer.data() + total_written, read - total_written), use_awaitable);
            }
        }
    }

    awaitable<std::string> read_file(const std::filesystem::path& file_path)
    {
        auto ex = co_await asio::this_coro::executor;

        asio::stream_file file(ex);
        file.open(file_path, asio::stream_file::read_only);

        std::array<char, 128> buffer{};
        std::string content;

        for (;;)
        {
            boost::system::error_code ec;
            const std::size_t read = co_await file.async_read_some(asio::buffer(buffer),
                asio::redirect_error(use_awaitable, ec));

            if (ec == asio::error::eof)
            {
                break;
            }

            if (ec)
            {
                throw boost::system::system_error(ec);
            }

            content.append(buffer.data(), read);
        }

        co_return content;
    }

    awaitable<void> run_example(const std::filesystem::path file_path)
    {
        const std::string text =
            "Boost.Asio coroutine example\n"
            "Writing lines asynchronously\n"
            "Reading them back\n";

        auto copy_path = file_path;
        copy_path += ".copy";

        co_await write_file(file_path, text);
        co_await copy_file_in_chunks(file_path, copy_path);

        const auto content = co_await read_file(copy_path);

        std::cout << "Copied file content from coroutine:\n" << content << std::flush;
    }
}

AWL_EXAMPLE(AsioStreamFile)
{
    AWL_UNUSED_CONTEXT;

    asio::io_context io;
    const auto temp_file = std::filesystem::temp_directory_path() / "asio_stream_file_example.txt";

    asio::co_spawn(io, run_example(temp_file), asio::detached);

    io.run();

    std::error_code ec;
    std::filesystem::remove(temp_file, ec);

    auto copied_file = temp_file;
    copied_file += ".copy";
    std::filesystem::remove(copied_file, ec);
}
