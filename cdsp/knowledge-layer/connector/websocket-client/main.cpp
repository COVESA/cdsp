#include <data_types.h>
#include <model_config.h>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <cstdlib>
#include <iostream>
#include <nlohmann/json.hpp>

#include "websocket_client.h"

using json = nlohmann::json;

boost::uuids::random_generator uuidGenerator;

std::string DefaultHostWebsocketServer{"127.0.0.1"};
std::string DefaultPortWebSocketServer{"8080"};
std::string ModelConfigurationFile{std::string(PROJECT_ROOT) +
                                   "/symbolic-reasoner/examples/use-case/model/model_config.json"};
std::string DefaultRDFoxServer{"127.0.0.1"};
std::string DefaultPortRDFoxServer{"12110"};
std::string DefaultAuthRDFoxServerBase64{"cm9vdDphZG1pbg=="};  // For 'root:admin' encoded in base64

std::string getEnvVariable(const std::string& envVar, const std::string& defaultValue = "") {
    const char* valueEnv = std::getenv(envVar.c_str());
    return valueEnv ? std::string(valueEnv) : defaultValue;
}

void displayHelp() {
    std::string bold = "\033[1m";
    std::string lightRed = "\033[91m";
    std::string reset = "\033[0m";

    std::cout << bold << "Usage: websocket_client [--help]\n\n" << reset;

    std::cout << "This table contains a lists environment variables set for the WebSocket client "
                 "and their descriptions.\n\n";
    // Table header
    std::cout << bold << std::left << std::setw(35) << "Variable" << std::setw(65) << "Description"
              << "Value" << reset << "\n";
    std::cout << std::string(140, '-') << "\n";  // Line separator

    std::cout << std::left << std::setw(35) << "HOST_WEBSOCKET_SERVER" << std::setw(65)
              << "IP address of the WebSocket server"
              << getEnvVariable("HOST_WEBSOCKET_SERVER", DefaultHostWebsocketServer) << "\n";
    std::cout << std::left << std::setw(35) << "PORT_WEBSOCKET_SERVER" << std::setw(65)
              << "Port number of the WebSocket server"
              << getEnvVariable("PORT_WEBSOCKET_SERVER", DefaultPortWebSocketServer) << "\n";
    std::cout << std::left << std::setw(35) << "OBJECT_ID" << std::setw(65)
              << "Object ID to be used in communication"
              << getEnvVariable("OBJECT_ID", lightRed + "`Not Set (Required)`" + reset) << "\n";
    std::cout << std::left << std::setw(35) << "HOST_RDFOX_SERVER" << std::setw(65)
              << "IP address of the RDFox server"
              << getEnvVariable("HOST_RDFOX_SERVER", DefaultRDFoxServer) << "\n";
    std::cout << std::left << std::setw(35) << "PORT_RDFOX_SERVER" << std::setw(65)
              << "Port number of the RDFox server"
              << getEnvVariable("PORT_RDFOX_SERVER", DefaultPortRDFoxServer) << "\n";
    std::cout << std::left << std::setw(35) << "AUTH_RDFOX_SERVER_BASE64" << std::setw(65)
              << "Authentication credentials for RDFox Server encoded in base64"
              << getEnvVariable("AUTH_RDFOX_SERVER_BASE64", DefaultAuthRDFoxServerBase64) << "\n";

    std::cout << "\n" << bold << "Description:\n" << reset;
    std::cout << "This client connects to a WebSocket server and processes incoming messages based "
                 "on the defined input.\n";
    std::cout << "The above environment variables are used to configure the application.\n\n";
}

/**
 * @brief Initializes and returns an InitConfig object with required configuration variables.
 *
 * This function creates an InitConfig instance and retrieving values
 * from environment variables. If the environment variables are not set,
 * default values are used.
 *
 * @return InitConfig The initialized configuration object.
 * @throws std::runtime_error if there is a problem setting a variable.
 */
InitConfig AddInitConfig() {
    ModelConfig model_config;
    loadModelConfig(ModelConfigurationFile, model_config);

    InitConfig init_config;
    init_config.websocket_server.host =
        getEnvVariable("HOST_WEBSOCKET_SERVER", DefaultHostWebsocketServer);
    init_config.websocket_server.port =
        getEnvVariable("PORT_WEBSOCKET_SERVER", DefaultPortWebSocketServer);
    init_config.uuid = boost::uuids::to_string(uuidGenerator());
    init_config.oid = getEnvVariable("OBJECT_ID");
    init_config.model_config = model_config;
    init_config.rdfox_server.host = getEnvVariable("HOST_RDFOX_SERVER");
    init_config.rdfox_server.port = getEnvVariable("PORT_RDFOX_SERVER");
    init_config.rdfox_server.auth_base64 = getEnvVariable("AUTH_RDFOX_SERVER_BASE64");
    return init_config;
}

int main(int argc, char* argv[]) {
    // Check for --help flag
    if (argc > 1 && std::string(argv[1]) == "--help") {
        displayHelp();
        return EXIT_SUCCESS;
    }
    try {
        InitConfig init_config = AddInitConfig();
        std ::cout << "** Starting client! **" << std::endl;
        auto client = std::make_shared<WebSocketClient>(init_config);
        client->run();

        return EXIT_SUCCESS;
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
