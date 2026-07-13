#include <boost/asio.hpp>
#include <iostream>
#include <print>

using boost::asio::ip::tcp;

bool server_listen(tcp::acceptor&, tcp::endpoint&, boost::system::error_code&);

boost::asio::awaitable<void> async_connection(tcp::acceptor&,
                                              boost::asio::io_context&, boost::system::error_code&);

boost::asio::awaitable<void> establish_connection(tcp::acceptor&, tcp::socket&, boost::system::error_code&);

boost::asio::awaitable<void> read_return_data(tcp::socket&, boost::system::error_code&);

int main() {
    boost::asio::io_context io_context;
    boost::system::error_code ec;
    tcp::acceptor acceptor(io_context);
    tcp::endpoint endpoint(boost::asio::ip::make_address("127.0.0.1", ec), 8080);

    if (ec || !server_listen(acceptor, endpoint, ec))
        return 1;

    boost::asio::co_spawn(io_context, async_connection(acceptor, io_context, ec), boost::asio::detached);

    io_context.run();
    return 0;
}

bool server_listen(tcp::acceptor& acceptor, tcp::endpoint& endpoint,
                   boost::system::error_code& ec) {
    acceptor.open(endpoint.protocol(), ec);
    acceptor.bind(endpoint, ec);
    acceptor.listen(boost::asio::socket_base::max_listen_connections, ec);

    if (ec) {
        std::print("Failed to start server: {}", ec.message());
        return false;
    }

    std::print("Server is listening on port 8080\n");
    return true;
}

boost::asio::awaitable<void> async_connection(
    tcp::acceptor& acceptor, boost::asio::io_context& io_context,
                                              boost::system::error_code& ec) {
    while (true) {
        tcp::socket socket(io_context);

        co_await establish_connection(acceptor, socket, ec);

        if (!ec) {
            co_await read_return_data(socket, ec);
        }
    }
}

boost::asio::awaitable<void> establish_connection(tcp::acceptor& acceptor, tcp::socket& socket,
                                                  boost::system::error_code& ec) {
    co_await acceptor.async_accept(socket,
                                   boost::asio::redirect_error(boost::asio::use_awaitable, ec));

    if (ec) {
        std::print("\nConnection failed: {}\n", ec.message());
    } else {
        std::print("\nClient successfully connected!\n");
    }
}

boost::asio::awaitable<void> read_return_data(tcp::socket& socket, boost::system::error_code& ec) {
    char data[1024];
    while (true) {
        size_t length = co_await socket.async_read_some(
            boost::asio::buffer(data), boost::asio::redirect_error(boost::asio::use_awaitable, ec));

        if (ec == boost::asio::error::eof) {
            std::print("Client disconnected without an error\n\n");
            break;
        } else if (ec) {
            std::print("Data error: {}\n\n", ec.message());
            break;
        }

        co_await boost::asio::async_write(
            socket, boost::asio::buffer(data, length),
            boost::asio::redirect_error(boost::asio::use_awaitable, ec));
    }
}