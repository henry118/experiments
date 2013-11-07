#include <iostream>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>

using boost::asio::ip::tcp;

struct session_t {
    boost::shared_ptr<tcp::socket> peer;
    std::vector<char> buf;
    session_t(boost::shared_ptr<tcp::socket> s) : peer(s), buf(1024, 0) {}
};

static void
initiate_cli(boost::asio::posix::stream_descriptor & input, boost::asio::streambuf & sb);
static void
initiate_accept(tcp::acceptor & acceptor);
static void
initiate_peer_read(boost::shared_ptr<session_t> sess);

static void
handle_peer_read(
    boost::shared_ptr<session_t> sess,
    const boost::system::error_code & error,
    std::size_t bytes_transferred)
{
    if (error) {
        std::cerr << "ERROR: " << error.message() << std::endl;
        sess->peer->close();
        return;
    }
    if (bytes_transferred > 0) {
        std::string s(static_cast<char*>(&sess->buf[0]));
        std::cout << "\tPeer: " << s << std::endl;
    }
    initiate_peer_read(sess);
}

static void
handle_cli_read(
    boost::asio::posix::stream_descriptor & input,
    boost::asio::streambuf & sb,
    const boost::system::error_code & error,
    std::size_t length)
{
    if (error) {
        std::cerr << error.message() << std::endl;
        return;
    }
    std::istream is(&sb);
    std::string s;
    std::getline(is, s);
    std::cout << "[" << length << "] " << s << std::endl;
    if (s == "exit") {
        input.get_io_service().stop();
        return;
    }
    initiate_cli(input, sb);
}

static void
handle_accept(
    tcp::acceptor & acceptor,
    boost::shared_ptr<tcp::socket> peer,
    const boost::system::error_code & error)
{
    if (error) {
        std::cerr << "ERROR: " << error.message() << std::endl;
        acceptor.close();
        return;
    }
    initiate_accept(acceptor);
    initiate_peer_read(boost::shared_ptr<session_t>(new session_t(peer)));
}

static void
initiate_cli(boost::asio::posix::stream_descriptor & input, boost::asio::streambuf & sb) {
    boost::asio::async_read_until(
        input, sb, "\n",
        boost::bind(&handle_cli_read,
                    boost::ref(input),
                    boost::ref(sb),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
}

static void
initiate_accept(tcp::acceptor & acceptor) {
    std::cerr << "start accepting ..." << std::endl;
    boost::shared_ptr<tcp::socket> peer(new tcp::socket(acceptor.get_io_service()));
    acceptor.async_accept(
        *peer,
        boost::bind(&handle_accept,
                    boost::ref(acceptor), peer,
                    boost::asio::placeholders::error));
}

static void
initiate_peer_read(boost::shared_ptr<session_t> sess) {
    std::cerr << "start reading peer ..." << std::endl;
    boost::asio::async_read(
        *sess->peer,
        boost::asio::buffer(sess->buf, 1024),
        boost::asio::transfer_at_least(1),
        boost::bind(
            &handle_peer_read,
            sess,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
}

int main() {
    boost::asio::io_service io;

    boost::asio::posix::stream_descriptor input(io, STDIN_FILENO);
    boost::asio::streambuf inputbuf;
    initiate_cli(input, inputbuf);

    tcp::endpoint endpoint(tcp::v4(), 5685);
    tcp::acceptor acceptor(io, endpoint);
    initiate_accept(acceptor);

    io.run();
    return 0;
}
