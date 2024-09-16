#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/system/error_code.hpp>
#include <nlohmann/json.hpp>
#include <string>

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = net::ip::tcp;
using json = nlohmann::json;

class WebSocketClient : public std::enable_shared_from_this<WebSocketClient> {
   public:
    WebSocketClient(const std::string& host, const std::string& port);
    void run();
    void sendMessage(const json& message);

   private:
    void onResolve(beast::error_code error_code, tcp::resolver::results_type results);
    void onConnect(boost::system::error_code error_code, tcp::resolver::iterator end_point);
    void handshake(beast::error_code error_code);
    void receiveMessage();
    void onReceiveMessage(beast::error_code error_code, std::size_t bytes_transferred);
    void onSendMessage(boost::system::error_code ec, std::size_t bytes_transferred);

   private:
    net::io_context io_context_;
    websocket::stream<tcp::socket> ws_;
    tcp::resolver resolver_;
    beast::flat_buffer buffer_;

    std::string server_uri_;
    std::string server_port_;
};