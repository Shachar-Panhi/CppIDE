#ifndef client_hpp
#define client_hpp

#include <boost/asio.hpp>
#include <expected>

namespace ClientNamespace {
    using boost::asio::ip::tcp;

    std::expected<tcp::socket, boost::system::error_code> connect_client(boost::asio::io_context&);
    boost::asio::awaitable<void> async_connection(tcp::socket);
    boost::asio::awaitable<std::expected<void, boost::system::error_code>> send_message(tcp::socket*);
    boost::asio::awaitable<void> receive_print_reply(tcp::socket*);
}

#endif