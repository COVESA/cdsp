#include <boost/asio.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <cstdlib>
#include <iostream>
#include <nlohmann/json.hpp>

#include "data_types.h"
#include "helper.h"
#include "model_config.h"
#include "reasoner_factory.h"
#include "reasoner_service.h"
#include "system_configuration_service.h"
#include "websocket_client.h"

using json = nlohmann::json;

boost::uuids::random_generator uuidGenerator;

constexpr char DEFAULT_HOST_WEB_SOCKET_SERVER[] = "127.0.0.1";
constexpr char DEFAULT_PORT_WEB_SOCKET_SERVER[] = "8080";
const std::string MODEL_CONFIGURATION_FILE =
    std::string(PROJECT_ROOT) + "/symbolic-reasoner/examples/use-case/model/model_config.json";
constexpr char DEFAULT_REASONER_SERVER[] = "127.0.0.1";
constexpr char DEFAULT_PORT_REASONER_SERVER[] = "12110";
constexpr char DEFAULT_AUTH_REASONER_SERVER_BASE64[] =
    "cm9vdDphZG1pbg==";  // 'root:admin' in base64
constexpr char DEFAULT_REASONER_DATASTORE_NAME[] = "ds-test";

void displayHelp() {
    std::string bold = "\033[1m";
    std::string light_red = "\033[91m";
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
              << Helper::getEnvVariable("HOST_WEBSOCKET_SERVER", DEFAULT_HOST_WEB_SOCKET_SERVER)
              << "\n";
    std::cout << std::left << std::setw(35) << "PORT_WEBSOCKET_SERVER" << std::setw(65)
              << "Port number of the WebSocket server"
              << Helper::getEnvVariable("PORT_WEBSOCKET_SERVER", DEFAULT_PORT_WEB_SOCKET_SERVER)
              << "\n";
    std::cout << std::left << std::setw(35) << "<SCHEMA_DEFINITION>_OBJECT_ID" << std::setw(65)
              << "Object ID to be used in communication, where <SCHEMA_DEFINITION> is the "
                 "uppercase schema type, e.g., VEHICLE_OBJECT_ID to set a VIN"
              << Helper::getEnvVariable("<SCHEMA_DEFINITION>_OBJECT_ID",
                                        light_red + "`Not Set (Required)`" + reset)
              << "\n";
    std::cout << std::left << std::setw(35) << "HOST_REASONER_SERVER" << std::setw(65)
              << "IP address of the reasoner server"
              << Helper::getEnvVariable("HOST_REASONER_SERVER", DEFAULT_REASONER_SERVER) << "\n";
    std::cout << std::left << std::setw(35) << "PORT_REASONER_SERVER" << std::setw(65)
              << "Port number of the reasoner server"
              << Helper::getEnvVariable("PORT_REASONER_SERVER", DEFAULT_PORT_REASONER_SERVER)
              << "\n";
    std::cout << std::left << std::setw(35) << "AUTH_REASONER_SERVER_BASE64" << std::setw(65)
              << "Authentication credentials for reasoner Server encoded in base64"
              << Helper::getEnvVariable("AUTH_REASONER_SERVER_BASE64",
                                        DEFAULT_AUTH_REASONER_SERVER_BASE64)
              << "\n";
    std::cout << std::left << std::setw(35) << "REASONER_DATASTORE_NAME" << std::setw(65)
              << "Datastore name of the reasoner server"
              << Helper::getEnvVariable("REASONER_DATASTORE_NAME", DEFAULT_REASONER_DATASTORE_NAME)
              << "\n";

    std::cout << "\n" << bold << "Description:\n" << reset;
    std::cout << "This client connects to a WebSocket server and processes incoming messages based "
                 "on the defined input.\n";
    std::cout << "The above environment variables are used to configure the application.\n\n";
}

int main(int argc, char* argv[]) {
    // Check for --help flag
    if (argc > 1 && std::string(argv[1]) == "--help") {
        displayHelp();
        return EXIT_SUCCESS;
    }
    try {
        // Initialize System Configuration
        SystemConfig system_config = SystemConfigurationService::loadSystemConfig(
            DEFAULT_HOST_WEB_SOCKET_SERVER, DEFAULT_PORT_WEB_SOCKET_SERVER, DEFAULT_REASONER_SERVER,
            DEFAULT_PORT_REASONER_SERVER, DEFAULT_AUTH_REASONER_SERVER_BASE64,
            DEFAULT_REASONER_DATASTORE_NAME);

        // Initialize Model Configuration
        std::shared_ptr<ModelConfig> model_config = std::make_shared<ModelConfig>(
            SystemConfigurationService::loadModelConfig(MODEL_CONFIGURATION_FILE));

        // Initialize Reasoner Service
        std::shared_ptr<ReasonerService> reasoner_service = ReasonerFactory::initReasoner(
            model_config->getReasonerSettings().getInferenceEngine(), system_config.reasoner_server,
            model_config->getReasonerRules());

        // Create the WebSocketClient
        std::cout << std::endl << "** Starting client! **" << std::endl;
        auto client =
            std::make_shared<WebSocketClient>(system_config, reasoner_service, model_config);

        // Initialize the RealWebSocketConnection with the WebSocketClient
        client->initializeConnection();

        // Run the WebSocket client
        client->run();

        return EXIT_SUCCESS;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}