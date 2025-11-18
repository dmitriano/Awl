#include "Awl/Testing/UnitTest.h"

#include <boost/asio.hpp>
#include <boost/asio/experimental/channel.hpp>
#include <boost/asio/stream_file.hpp>
#include <iostream>
#include <fstream>

namespace
{
    using boost::asio::awaitable;
    using boost::asio::use_awaitable;
    using boost::asio::as_tuple;
    namespace this_coro = boost::asio::this_coro;

    constexpr std::size_t BLOCK = 64 * 1024;

    using Chunk = std::shared_ptr<std::vector<char>>;
    using Channel = boost::asio::experimental::channel<void(boost::system::error_code, Chunk)>;

    // ====================================================================
    // READER: read file and send chunks to reader_chan
    // ====================================================================
    awaitable<void> reader(const std::string& src, Channel& reader_chan)
    {
        std::ifstream in(src, std::ios::binary);
        if (!in.is_open()) {
            co_await reader_chan.async_send(
                make_error_code(boost::system::errc::no_such_file_or_directory),
                nullptr,
                use_awaitable);
            reader_chan.close();
            co_return;
        }

        for (;;) {
            auto buf = std::make_shared<std::vector<char>>(BLOCK);
            in.read(buf->data(), buf->size());
            std::streamsize n = in.gcount();

            if (n <= 0)
                break;

            buf->resize(static_cast<std::size_t>(n));

            std::cout << "reader: block size = " << buf->size() << " bytes\n";

            co_await reader_chan.async_send({}, buf, use_awaitable);

            if (in.eof())
                break;
        }

        reader_chan.close();
    }

    // ====================================================================
    // HANDLER: receive chunk from reader_chan, print size, send to writer_chan
    // ====================================================================
    awaitable<void> handler(Channel& reader_chan, Channel& writer_chan)
    {
        for (;;) {
            auto [ec, chunk] =
                co_await reader_chan.async_receive(as_tuple(use_awaitable));

            if (ec == boost::asio::experimental::channel_errc::channel_closed)
                break;

            if (ec) {
                std::cout << "reader error: " << ec.message() << "\n";
                break;
            }

            std::cout << "handler: block size = " << chunk->size() << " bytes\n";

            co_await writer_chan.async_send({}, chunk, use_awaitable);
        }

        writer_chan.close();
    }

    // ====================================================================
    // WRITER: receive chunk from writer_chan and write to file
    // ====================================================================
    awaitable<void> writer(const std::string& dst, Channel& writer_chan)
    {
        std::ofstream out(dst, std::ios::binary);
        if (!out.is_open()) {
            std::cout << "Cannot open output file\n";
            co_return;
        }

        for (;;) {
            auto [ec, chunk] =
                co_await writer_chan.async_receive(as_tuple(use_awaitable));

            if (ec == boost::asio::experimental::channel_errc::channel_closed)
                break;

            if (ec) {
                std::cout << "handler error: " << ec.message() << "\n";
                break;
            }

            out.write(chunk->data(), chunk->size());
        }

        out.flush();
    }

    // ====================================================================
    // Pipeline: reader -> handler -> writer
    // ====================================================================
    awaitable<void> run_pipeline(const std::string& src, const std::string& dst)
    {
        auto exec = co_await this_coro::executor;

        Channel reader_chan(exec, 5);

        Channel writer_chan(exec, 3);

        co_spawn(exec, reader(src, reader_chan), boost::asio::detached);
        // co_spawn(exec, handler(reader_chan, writer_chan), boost::asio::detached);
        // co_spawn(exec, writer(dst, writer_chan), boost::asio::detached);
    }
}

AWL_EXAMPLE(CopyFileWithHandler)
{
    AWL_ATTRIBUTE(std::string, src, "input.dat");
    AWL_ATTRIBUTE(std::string, dst, "output.dat");

    try
    {
        boost::asio::io_context io;
        co_spawn(io, run_pipeline(src, dst), boost::asio::detached);
        io.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
}
