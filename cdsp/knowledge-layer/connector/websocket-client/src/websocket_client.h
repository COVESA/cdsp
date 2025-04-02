#ifndef WEBSOCKET_CLIENT_H
#define WEBSOCKET_CLIENT_H

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "data_types.h"
#include "file_handler_impl.h"
#include "json_writer.h"
#include "message_service.h"
#include "model_config.h"
#include "reasoner_service.h"
#include "reasoning_query_service.h"
#include "triple_assembler.h"
#include "triple_writer.h"
#include "websocket_interface.h"

namespace beast = boost::beast;
namespace net = boost::asio;
using tcp = net::ip::tcp;
using json = nlohmann::json;

class WebSocketClient : public std::enable_shared_from_this<WebSocketClient> {
   public:
    WebSocketClient(const SystemConfig& system_config, std::shared_ptr<ModelConfig> model_config,
                    std::shared_ptr<ReasonerService> reasoner_service,
                    std::shared_ptr<WebSocketClientInterface> connection = nullptr);

    void initializeConnection();
    void run();
    void sendMessage(const json& message);
    const SystemConfig& getInitConfig() const;
    void onConnect(boost::system::error_code ec, const boost::asio::ip::tcp::endpoint& endpoint);
    void handshake(boost::system::error_code ec);
    void onSendMessage(boost::system::error_code ec, std::size_t bytes_transferred);
    void onReceiveMessage(beast::error_code ec, std::size_t bytes_transferred);

   private:
    SystemConfig system_config_;
    net::io_context io_context_;
    std::shared_ptr<WebSocketClientInterface> connection_;
    std::shared_ptr<ReasonerService> reasoner_service_;
    std::shared_ptr<ModelConfig> model_config_;
    TripleAssembler triple_assembler_;
    std::shared_ptr<ReasoningQueryService> reasoner_query_service_;
    FileHandlerImpl file_handler_;
    TripleWriter triple_writer_;
    MessageService message_service_;

    std::vector<json> reply_messages_queue_;
    std::vector<std::shared_ptr<const std::string>> response_messages_queue_;

    void processMessage(const std::shared_ptr<const std::string>& message);
    void writeReplyMessagesOnQueue();
};

#endif  // WEBSOCKET_CLIENT_H