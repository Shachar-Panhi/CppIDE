#include <boost/asio.hpp>
#include <iostream>
#include <print>

using boost::asio::ip::tcp;

bool server_listen(tcp::acceptor&, tcp::endpoint&, boost::system::error_code&);
void establish_connection(tcp::acceptor&, tcp::socket&, boost::system::error_code&);
void read_return_data(tcp::socket&, boost::system::error_code&);

int main() {
    boost::asio::io_context io_context;
    boost::system::error_code ec;
    tcp::acceptor acceptor(io_context);
    tcp::endpoint endpoint(boost::asio::ip::make_address("127.0.0.1"), 8080);

    if (!server_listen(acceptor, endpoint, ec))
        return 1;

    while (true) {
        tcp::socket socket(io_context);
        establish_connection(acceptor, socket, ec);

        if (!ec) {
            read_return_data(socket, ec);
        }
    }
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

void establish_connection(tcp::acceptor& acceptor, tcp::socket& socket,
                          boost::system::error_code& ec) {
    acceptor.accept(socket, ec);

    if (ec) {
        std::print("Connection failed: {}\n", ec.message());
    } else {
        std::print("Client successfully connected!\n");
    }
}

void read_return_data(tcp::socket& socket, boost::system::error_code& ec) {
    char data[1024];
    while (true) {
        size_t length = socket.read_some(boost::asio::buffer(data), ec);

        if (ec == boost::asio::error::eof) {
            std::print("Client disconnected without an error\n");
            break;
        } else if (ec) {
            std::print("Data error: {}\n", ec.message());
            break;
        }

        boost::asio::write(socket, boost::asio::buffer(data, length), ec);
    }
}