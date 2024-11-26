#ifndef WEBSOCKET_INTERFACE_H
#define WEBSOCKET_INTERFACE_H

#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

class WebSocketClientInterface {
   public:
    virtual ~WebSocketClientInterface() = default;

    /**
     * @brief Resolve the WebSocket server's host and port.
     *
     * @param host The WebSocket server's hostname or IP address.
     * @param port The WebSocket server's port.
     */
    virtual void asyncResolve(const std::string& host, const std::string& port) = 0;

    /**
     * @brief Establish a connection to the resolved host and port.
     */
    virtual void asyncConnect() = 0;

    /**
     * @brief Perform the WebSocket handshake.
     */
    virtual void asyncHandshake() = 0;

    /**
     * @brief Send a JSON message asynchronously.
     *
     * @param message The JSON message to send.
     */
    virtual void asyncWrite(const json& message) = 0;

    /**
     * @brief Start asynchronously reading messages.
     */
    virtual void asyncRead() = 0;

    /**
     * @brief Retrieve the most recent message received from the WebSocket.
     *
     * @return The received message as a string.
     */
    virtual std::string getReceivedMessage() = 0;

    /**
     * @brief Consume (clear) a portion of the WebSocket's buffer after reading.
     *
     * @param bytes_transferred The number of bytes to consume.
     */
    virtual void consumeBuffer(std::size_t bytes) = 0;
};

#endif  // WEBSOCKET_INTERFACE_H