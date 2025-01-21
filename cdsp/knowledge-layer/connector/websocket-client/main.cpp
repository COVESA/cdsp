#include <boost/asio.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <cstdlib>
#include <iostream>
#include <nlohmann/json.hpp>

#include "file_handler_impl.h"
#include "model_config_utils.h"
#include "triple_assembler.h"
#include "triple_writer.h"
#include "websocket_client.h"

using json = nlohmann::json;

boost::uuids::random_generator uuidGenerator;

constexpr char DEFAULT_HOST_WEB_SOCKET_SERVER[] = "127.0.0.1";
constexpr char DEFAULT_PORT_WEB_SOCKET_SERVER[] = "8080";
const std::string MODEL_CONFIGURATION_FILE =
    std::string(PROJECT_ROOT) + "/symbolic-reasoner/examples/use-case/model/model_config.json";
constexpr char DEFAULT_RDFOX_SERVER[] = "127.0.0.1";
constexpr char DEFAULT_PORT_RDFOX_SERVER[] = "12110";
constexpr char DEFAULT_AUTH_RDFOX_SERVER_BASE64[] = "cm9vdDphZG1pbg==";  // 'root:admin' in base64
constexpr char DEFAULT_RDFOX_DATASTORE[] = "ds-test";

std::string getEnvVariable(const std::string& env_var, const std::string& default_value = "") {
    const char* value_env = std::getenv(env_var.c_str());
    return value_env ? std::string(value_env) : default_value;
}

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
              << getEnvVariable("HOST_WEBSOCKET_SERVER", DEFAULT_HOST_WEB_SOCKET_SERVER) << "\n";
    std::cout << std::left << std::setw(35) << "PORT_WEBSOCKET_SERVER" << std::setw(65)
              << "Port number of the WebSocket server"
              << getEnvVariable("PORT_WEBSOCKET_SERVER", DEFAULT_PORT_WEB_SOCKET_SERVER) << "\n";
    std::cout << std::left << std::setw(35) << "OBJECT_ID" << std::setw(65)
              << "Object ID to be used in communication"
              << getEnvVariable("OBJECT_ID", light_red + "`Not Set (Required)`" + reset) << "\n";
    std::cout << std::left << std::setw(35) << "HOST_RDFOX_SERVER" << std::setw(65)
              << "IP address of the RDFox server"
              << getEnvVariable("HOST_RDFOX_SERVER", DEFAULT_RDFOX_SERVER) << "\n";
    std::cout << std::left << std::setw(35) << "PORT_RDFOX_SERVER" << std::setw(65)
              << "Port number of the RDFox server"
              << getEnvVariable("PORT_RDFOX_SERVER", DEFAULT_PORT_RDFOX_SERVER) << "\n";
    std::cout << std::left << std::setw(35) << "AUTH_RDFOX_SERVER_BASE64" << std::setw(65)
              << "Authentication credentials for RDFox Server encoded in base64"
              << getEnvVariable("AUTH_RDFOX_SERVER_BASE64", DEFAULT_AUTH_RDFOX_SERVER_BASE64)
              << "\n";
    std::cout << std::left << std::setw(35) << "RDFOX_DATASTORE" << std::setw(65)
              << "Datastore name of the RDFox server"
              << getEnvVariable("RDFOX_DATASTORE", DEFAULT_RDFOX_DATASTORE) << "\n";

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
    ModelConfigUtils::loadModelConfig(MODEL_CONFIGURATION_FILE, model_config);

    InitConfig init_config;
    init_config.websocket_server.host =
        getEnvVariable("HOST_WEBSOCKET_SERVER", DEFAULT_HOST_WEB_SOCKET_SERVER);
    init_config.websocket_server.port =
        getEnvVariable("PORT_WEBSOCKET_SERVER", DEFAULT_PORT_WEB_SOCKET_SERVER);
    init_config.uuid = boost::uuids::to_string(uuidGenerator());
    init_config.oid = getEnvVariable("OBJECT_ID");
    init_config.model_config = model_config;
    init_config.rdfox_server.host = getEnvVariable("HOST_RDFOX_SERVER", DEFAULT_RDFOX_SERVER);
    init_config.rdfox_server.port = getEnvVariable("PORT_RDFOX_SERVER", DEFAULT_PORT_RDFOX_SERVER);
    init_config.rdfox_server.auth_base64 =
        getEnvVariable("AUTH_RDFOX_SERVER_BASE64", DEFAULT_AUTH_RDFOX_SERVER_BASE64);
    init_config.rdfox_server.data_store =
        getEnvVariable("RDFOX_DATASTORE", DEFAULT_RDFOX_DATASTORE);
    return init_config;
}

int main(int argc, char* argv[]) {
    // Check for --help flag
    if (argc > 1 && std::string(argv[1]) == "--help") {
        displayHelp();
        return EXIT_SUCCESS;
    }
    try {
        // Initialize configuration
        InitConfig init_config = AddInitConfig();

        // Initialize TripleAssembler
        RDFoxAdapter rdfox_adapter(init_config.rdfox_server.host, init_config.rdfox_server.port,
                                   init_config.rdfox_server.auth_base64,
                                   init_config.rdfox_server.data_store.value());
        rdfox_adapter.initialize();

        // Initialize TripleAssembler
        FileHandlerImpl file_handler;
        TripleWriter triple_writer;
        TripleAssembler triple_assembler(init_config.model_config, rdfox_adapter, file_handler,
                                         triple_writer);
        triple_assembler.initialize();

        std::cout << std::endl << "** Starting client! **" << std::endl;

        // Create the WebSocketClient
        auto client = std::make_shared<WebSocketClient>(init_config, triple_assembler);

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