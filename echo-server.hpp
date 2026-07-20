#ifndef server_hpp
#define server_hpp

#include <boost/asio.hpp>
#include <expected>

namespace ServerNamespace {
    using boost::asio::ip::tcp;

    std::expected<tcp::acceptor, boost::system::error_code> server_listen(boost::asio::io_context&);
    boost::asio::awaitable<void> async_connection(tcp::acceptor);
    boost::asio::awaitable<std::expected<tcp::socket, boost::system::error_code>> establish_connection(tcp::acceptor*);
    boost::asio::awaitable<void> read_return_data(tcp::socket);

}

#endif