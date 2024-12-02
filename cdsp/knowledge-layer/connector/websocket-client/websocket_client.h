#ifndef WEBSOCKET_CLIENT_H
#define WEBSOCKET_CLIENT_H

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "data_types.h"
#include "message_utils.h"
#include "triple_assembler.h"
#include "websocket_interface.h"

namespace beast = boost::beast;
namespace net = boost::asio;
using tcp = net::ip::tcp;
using json = nlohmann::json;

class WebSocketClient : public std::enable_shared_from_this<WebSocketClient> {
   public:
    WebSocketClient(const InitConfig& init_config, TripleAssembler& triple_assembler,
                    std::shared_ptr<WebSocketClientInterface> connection = nullptr);

    void initializeConnection();
    void run();
    void sendMessage(const json& message);
    const InitConfig& getInitConfig() const;
    void onConnect(boost::system::error_code ec, tcp::resolver::iterator iterator);
    void handshake(boost::system::error_code ec);
    void onSendMessage(boost::system::error_code ec, std::size_t bytes_transferred);
    void onReceiveMessage(beast::error_code ec, std::size_t bytes_transferred);

   private:
    TripleAssembler& triple_assembler_;
    InitConfig init_config_;
    net::io_context io_context_;

    std::vector<json> reply_messages_queue_;
    std::vector<std::shared_ptr<const std::string>> response_messages_queue_;

    std::shared_ptr<WebSocketClientInterface> connection_;

    void processMessage(const std::shared_ptr<const std::string>& message);
    void writeReplyMessagesOnQueue();
};

#endif  // WEBSOCKET_CLIENT_H