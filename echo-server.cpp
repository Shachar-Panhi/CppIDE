#include "echo-server.hpp"
#include "boost/asio/error.hpp"
#include <print>
#include <array>

namespace ServerNamespace {
    std::expected<tcp::acceptor, boost::system::error_code> server_listen(boost::asio::io_context& io_context) {
        boost::system::error_code ec;

        tcp::acceptor acceptor(io_context);

        tcp::endpoint endpoint(boost::asio::ip::make_address("127.0.0.1", ec), 8080);
        if (ec){
            std::println("invalid ip address {}", ec.message());
            return std::unexpected(ec);
        }

        acceptor.open(endpoint.protocol(), ec);
        acceptor.bind(endpoint, ec);
        acceptor.listen(boost::asio::socket_base::max_listen_connections, ec);

        if (ec) {
            std::print("Failed to start server: {}", ec.message());
            return std::unexpected(ec);
        }

        std::print("Server is listening on port 8080\n");
        return acceptor;
    }

    boost::asio::awaitable<void> async_connection(tcp::acceptor acceptor) {
        auto context_engine = co_await boost::asio::this_coro::executor;

        while (true) {
            auto result = co_await establish_connection(&acceptor);

            if (result.has_value()) {
                boost::asio::co_spawn(context_engine, read_return_data(std::move(result.value())),
                                    boost::asio::detached);
            }
        }
    }

    boost::asio::awaitable<std::expected<tcp::socket, boost::system::error_code>> establish_connection(tcp::acceptor* acceptor) {
        boost::system::error_code ec;
        
        tcp::socket socket = co_await acceptor->async_accept(boost::asio::redirect_error(boost::asio::use_awaitable, ec));

        if (ec) {
            std::print("\nConnection failed: {}\n", ec.message());
            co_return std::unexpected(ec);
        }
        std::print("\nClient successfully connected!\n");

        co_return socket;
    }

    boost::asio::awaitable<void> read_return_data(tcp::socket socket) {
        boost::system::error_code ec;
        
        std::array<uint8_t, 1024> data{};
        while (true) {
            size_t length = co_await socket.async_read_some(
                boost::asio::buffer(data), boost::asio::redirect_error(boost::asio::use_awaitable, ec));

            if (ec == boost::asio::error::eof) {
                std::println("Client disconnected without an error\n");
                break; 
            }

            if (ec == boost::asio::error::operation_aborted) {
                std::println("Operation aborted\n");
                break;
            }
            
            if (ec) {
                std::println("Data error: {}\n", ec.message());
                break;
            }

            co_await boost::asio::async_write(
                socket, boost::asio::buffer(data, length),
                boost::asio::redirect_error(boost::asio::use_awaitable, ec));
        }
    }
}



int main() {
    boost::asio::io_context io_context;
    
    auto result = ServerNamespace::server_listen(io_context);
    if (!result.has_value()) {
        std::println("Connection error occurred");
        return 1;
    }

    boost::asio::co_spawn(io_context, ServerNamespace::async_connection(std::move(result.value())),
                          boost::asio::detached);

    io_context.run();
    return 0;
}