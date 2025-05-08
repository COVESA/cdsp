#ifndef MOCK_WEBSOCKET_CONNECTION_H
#define MOCK_WEBSOCKET_CONNECTION_H

#include <functional>
#include <queue>
#include <string>

#include "websocket_interface.h"

class MockWebSocketConnection : public WebSocketClientInterface {
   public:
    MockWebSocketConnection() : resolved_(false), connected_(false) {};

    void asyncResolve(const std::string& host, const std::string& port) override {
        resolved_ = true;
        if (onResolveCallback_) {
            onResolveCallback_();
        }
    }

    void asyncConnect() override {
        if (resolved_) {
            connected_ = true;
            if (onConnectCallback_)
                onConnectCallback_();
        }
    }

    void asyncHandshake() override {
        if (connected_) {
            if (onHandshakeCallback_)
                onHandshakeCallback_();
        }
    }

    void asyncWrite(const json& message) override {
        sent_messages_.push(message.dump());
        if (onWriteCallback_)
            onWriteCallback_(message.dump());
    }

    void asyncRead() override {
        if (!received_messages_.empty()) {
            buffer_ = received_messages_.front();
            received_messages_.pop();
            if (onReadCallback_)
                onReadCallback_();
        }
    }

    std::string getReceivedMessage() override { return buffer_; }

    void consumeBuffer(std::size_t bytes) override {
        // Mock: no-op
    }

    // Test setup methods
    void addReceivedMessage(const std::string& message) { received_messages_.push(message); }

    void setOnResolveCallback(std::function<void()> callback) { onResolveCallback_ = callback; }

    void setOnConnectCallback(std::function<void()> callback) { onConnectCallback_ = callback; }

    void setOnHandshakeCallback(std::function<void()> callback) { onHandshakeCallback_ = callback; }

    void setOnWriteCallback(std::function<void(const std::string&)> callback) {
        onWriteCallback_ = callback;
    }

    void setOnReadCallback(std::function<void()> callback) { onReadCallback_ = callback; }

   private:
    bool resolved_;
    bool connected_;
    std::queue<std::string> received_messages_;
    std::queue<std::string> sent_messages_;
    std::string buffer_;

    std::function<void()> onResolveCallback_;
    std::function<void()> onConnectCallback_;
    std::function<void()> onHandshakeCallback_;
    std::function<void(const std::string&)> onWriteCallback_;
    std::function<void()> onReadCallback_;
};

#endif  // MOCK_WEBSOCKET_CONNECTION_H