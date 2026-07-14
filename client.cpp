#include <boost/asio.hpp>
#include <iostream>
#include <print>

using boost::asio::ip::tcp;

bool connect_client(tcp::resolver&, tcp::socket&, boost::system::error_code&);
boost::asio::awaitable<void> async_connection(tcp::socket&, boost::system::error_code&);
boost::asio::awaitable<void> send_message(tcp::socket&, boost::system::error_code&);
boost::asio::awaitable<void> receive_print_reply(tcp::socket&, boost::system::error_code&);

int main() {
    boost::asio::io_context io_context;
    boost::system::error_code ec;
    tcp::resolver resolver(io_context);
    tcp::socket socket(io_context);

    if (!connect_client(resolver, socket, ec))
        return 1;

    boost::asio::co_spawn(io_context, async_connection(socket, ec), boost::asio::detached);

    io_context.run();
    return 0;
}

bool connect_client(tcp::resolver& resolver, tcp::socket& socket, boost::system::error_code& ec) {
    auto endpoints = resolver.resolve("127.0.0.1", "8080", ec);

    if (ec) {
        std::print("Resolver error encountered: {}", ec.message());
        return false;
    }

    boost::asio::connect(socket, endpoints, ec);

    if (ec) {
        std::print("Connection failed: {}", ec.message());
        return false;
    }

    std::print("Successfully connected!\n");
    return true;
}

boost::asio::awaitable<void> async_connection(tcp::socket& socket, boost::system::error_code& ec) {
    while (true) {
        co_await send_message(socket, ec);

        if (!ec)
            co_await receive_print_reply(socket, ec);
    }
}

boost::asio::awaitable<void> send_message(tcp::socket& socket, boost::system::error_code& ec) {
    std::print("Please enter a message: \n");
    std::string user_input;
    std::getline(std::cin, user_input);

    co_await boost::asio::async_write(socket, boost::asio::buffer(user_input), boost::asio::redirect_error(boost::asio::use_awaitable, ec));
    
    if (ec) 
        std::print("Send failed: {}\n", ec.message());
    else
        std::print("\nSent: {}\n", user_input);
}

boost::asio::awaitable<void> receive_print_reply(tcp::socket& socket,
                                                 boost::system::error_code& ec) {
    char reply[1024];

    size_t reply_length = co_await socket.async_read_some(boost::asio::buffer(reply),boost::asio::redirect_error(boost::asio::use_awaitable, ec));

    if (ec) 
        std::print("Receive failed: {}\n", ec.message());
    else {
        std::string_view received_message(reply, reply_length);
        std::print("Received back: {}\n\n", received_message);
    }
}