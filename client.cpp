#include "client.hpp"
#include <iostream>
#include <print>
#include <array>

namespace ClientNamespace {
    std::expected<tcp::socket, boost::system::error_code> connect_client(boost::asio::io_context& io_context) {
        boost::system::error_code ec;

        tcp::resolver resolver(io_context);
        tcp::socket socket(io_context);

        auto endpoints = resolver.resolve("127.0.0.1", "8080", ec);
        if (ec) {
            std::print("Resolver error encountered: {}", ec.message());
            return std::unexpected(ec);
        }

        boost::asio::connect(socket, endpoints, ec);

        if (ec) {
            std::println("Connection failed: {}", ec.message());
            return std::unexpected(ec);
        }

        std::println("Successfully connected!");
        return socket;
    }

    boost::asio::awaitable<void> async_connection(tcp::socket socket) {
        while (true) {
            auto result = co_await send_message(&socket);

            if (result.has_value()) {
                co_await receive_print_reply(&socket);
            }
        }
    }

    boost::asio::awaitable<std::expected<void, boost::system::error_code>> send_message(tcp::socket* socket) {
        boost::system::error_code ec;
        
        std::println("Please enter a message: ");
        std::string user_input;
        std::getline(std::cin, user_input);

        co_await boost::asio::async_write(*socket, boost::asio::buffer(user_input),
                                        boost::asio::redirect_error(boost::asio::use_awaitable, ec));

        if (ec) {
            std::println("Send failed: {}", ec.message());
            co_return std::unexpected(ec);
        }
        std::println("\nSent: {}", user_input);
        co_return std::expected<void, boost::system::error_code>{};
    }

    boost::asio::awaitable<void> receive_print_reply(tcp::socket* socket) {
        boost::system::error_code ec;

        std::array<char, 1024> reply{};

        size_t reply_length = co_await socket->async_read_some(
            boost::asio::buffer(reply, 1024),
            boost::asio::redirect_error(boost::asio::use_awaitable, ec));

        if (ec) {
            std::print("Receive failed: {}\n", ec.message());
        } else {
            std::string_view received_message(reply.data(), reply_length);
            std::println("Received back: {}\n", received_message);
        }
    }
}

int main() {
        boost::asio::io_context io_context;

        auto result = ClientNamespace::connect_client(io_context);
        if (!result.has_value()) {
            std::println("Connection error occurred");
            return 1;
        }

        boost::asio::co_spawn(io_context, ClientNamespace::async_connection(std::move(result.value())), boost::asio::detached);

        io_context.run();
        return 0;
    }