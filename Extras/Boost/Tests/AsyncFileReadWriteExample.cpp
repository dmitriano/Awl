#include <boost/asio.hpp>
#include <boost/asio/redirect_error.hpp>
#include <boost/asio/stream_file.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/system/system_error.hpp>

#include <array>
#include <filesystem>

namespace asio = boost::asio;
using asio::awaitable;
using asio::use_awaitable;

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
