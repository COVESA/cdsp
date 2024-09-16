#include <cstdlib>
#include <iostream>
#include <nlohmann/json.hpp>

#include "websocket_client.hpp"

using json = nlohmann::json;

std::string DefaultHostWebsocketServer{"127.0.0.1"};
std::string DefaultPortWebSocketServer{"8080"};

int main() {
    // Read the Websocket-Server HOST and PORT from the environment variables
    const char* hostEnv = std::getenv("HOST_WEBSOCKET_SERVER");
    const char* portEnv = std::getenv("PORT_WEBSOCKET_SERVER");

    std::string HostWebsocketServer = hostEnv ? hostEnv : DefaultHostWebsocketServer;
    std::string PortWebSocketServer = portEnv ? portEnv : DefaultPortWebSocketServer;

    std::cout << "** Starting client! **" << std::endl;
    auto client = std::make_shared<WebSocketClient>(HostWebsocketServer, PortWebSocketServer);
    client->run();

    return 0;
}