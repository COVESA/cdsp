#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/system/error_code.hpp>
#include <chrono>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "data_types.h"
#include "message_utils.h"

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = net::ip::tcp;
using json = nlohmann::json;

class WebSocketClient : public std::enable_shared_from_this<WebSocketClient> {
   public:
    WebSocketClient(const InitConfig& init_config);
    void run();
    void sendMessage(const json& message);

   private:
    void onResolve(beast::error_code error_code, tcp::resolver::results_type results);
    void onConnect(boost::system::error_code error_code, tcp::resolver::iterator end_point);
    void handshake(beast::error_code error_code);
    void receiveMessage();
    void onReceiveMessage(beast::error_code error_code, std::size_t bytes_transferred);
    void onSendMessage(boost::system::error_code ec, std::size_t bytes_transferred);
    void processMessage(std::shared_ptr<std::string const> const& ss);
    void onProcessMessage(std::shared_ptr<std::string const> const& ss);

    void writeReplyMessagesOnQueue();

    std::vector<json> reply_messages_queue_{};
    std::vector<std::shared_ptr<std::string const>> response_messages_queue_{};
    net::io_context io_context_;
    websocket::stream<tcp::socket> ws_;
    tcp::resolver resolver_;
    beast::flat_buffer buffer_;

    InitConfig init_config_;
};