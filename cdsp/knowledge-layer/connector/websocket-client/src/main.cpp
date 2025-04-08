#include <boost/asio.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <cstdlib>
#include <iostream>
#include <nlohmann/json.hpp>

#include "data_types.h"
#include "globals.h"
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
const std::string DEFAULT_TARGET_WEB_SOCKET_SERVER = "";
const std::string MODEL_CONFIGURATION_FILE =
    getProjectRoot() + "/symbolic-reasoner/examples/use-case/model/model_config.json";
constexpr char DEFAULT_REASONER_SERVER[] = "127.0.0.1";
constexpr char DEFAULT_PORT_REASONER_SERVER[] = "12110";
constexpr char DEFAULT_AUTH_REASONER_SERVER_BASE64[] =
    "cm9vdDphZG1pbg==";  // 'root:admin' in base64
constexpr char DEFAULT_REASONER_DATASTORE_NAME[] = "ds-test";
bool RESET_REASONER_DATASTORE = false;

void printBanner() {
    std::cout << "\033[1;36m"  // Bright blue color
              << "\n"
              << "╭───────────────────────────────────────────╮\n"
              << " ---------     REASONER CLIENT     --------- \n"
              << "╰───────────────────────────────────────────╯\n"
              << "\033[0m\n";  // Reset color
}

void displayHelp() {
    std::string bold = "\033[1m";
    std::string light_yellow = "\033[1;33m";
    std::string reset = "\033[0m";

    std::cout << bold << "Description:\n" << reset;
    std::cout << "-------------------------------------------------------------------------\n"
              << "A semantic reasoning engine that communicates via\n"
              << "WebSocket, receives structured messages, performs semantic\n"
              << "reasoning over RDF data, and returns intelligent responses.\n"
              << "Ideal for data-driven vehicle systems and knowledge-enabled environments.\n"
              << "\n";

    std::cout << "\nUsage: " << light_yellow
              << "<SCHEMA_DEFINITION>_OBJECT_ID=OBJECT_ID ./reasoner_client [options]" << reset
              << "\n";
    // Options
    std::cout << "\nThe following options are available:\n";

    std::cout << bold << std::left << std::setw(35) << "-X [opt]" << std::setw(65)
              << ": set implementation-specific option." << reset << "\n";
    std::cout << bold << std::left << std::setw(35) << "--help-env" << std::setw(65)
              << ": show environment variables." << reset << "\n";
    std::cout << bold << std::left << std::setw(35) << "--help-xoptions" << std::setw(65)
              << ": show implementation-specific options." << reset << "\n";
}

void displayEnvVariables() {
    std::string bold = "\033[1m";
    std::string light_yellow = "\033[1;33m";
    std::string reset = "\033[0m";
    std::cout << bold << "Environment Variables:\n" << reset;
    std::cout
        << "The following environment variables are used to configure the WebSocket client:\n\n";

    // Table Required Variables
    std::cout << bold << std::left << std::setw(35) << "Required Variables" << std::setw(65)
              << "Description" << reset << "\n";

    std::cout << std::string(140, '-') << "\n";  // Line separator

    std::cout << std::left << std::setw(35) << "<SCHEMA_DEFINITION>_OBJECT_ID" << std::setw(65)
              << "Object ID to be used in communication, where <SCHEMA_DEFINITION> is the "
                 "uppercase schema type, e.g.:\n"
              << std::setw(35) << "" << "VEHICLE_OBJECT_ID=VIN1234567891234\n\n";

    // Table Optional Variables
    std::cout << bold << std::left << std::setw(35) << "Variable" << std::setw(65) << "Description"
              << std::setw(40) << "Default Value" << reset << "\n";

    std::cout << std::string(145, '-') << "\n";  // Line separator

    std::cout << std::left << std::setw(35) << "HOST_WEBSOCKET_SERVER" << std::setw(65)
              << "IP address of the WebSocket server" << std::setw(40)
              << Helper::getEnvVariable("HOST_WEBSOCKET_SERVER", DEFAULT_HOST_WEB_SOCKET_SERVER)
              << "\n";

    std::cout << std::left << std::setw(35) << "PORT_WEBSOCKET_SERVER" << std::setw(65)
              << "Port number of the WebSocket server" << std::setw(40)
              << Helper::getEnvVariable("PORT_WEBSOCKET_SERVER", DEFAULT_PORT_WEB_SOCKET_SERVER)
              << "\n";

    std::cout << std::left << std::setw(35) << "TARGET_WEBSOCKET_SERVER" << std::setw(65)
              << "Target URL of the WebSocket server" << std::setw(40)
              << Helper::getEnvVariable("TARGET_WEBSOCKET_SERVER", DEFAULT_TARGET_WEB_SOCKET_SERVER)
              << "\n";

    std::cout << std::left << std::setw(35) << "HOST_REASONER_SERVER" << std::setw(65)
              << "IP address of the reasoner server" << std::setw(40)
              << Helper::getEnvVariable("HOST_REASONER_SERVER", DEFAULT_REASONER_SERVER) << "\n";

    std::cout << std::left << std::setw(35) << "PORT_REASONER_SERVER" << std::setw(65)
              << "Port number of the reasoner server" << std::setw(40)
              << Helper::getEnvVariable("PORT_REASONER_SERVER", DEFAULT_PORT_REASONER_SERVER)
              << "\n";

    std::cout << std::left << std::setw(35) << "AUTH_REASONER_SERVER_BASE64" << std::setw(65)
              << "Authentication credentials for reasoner Server encoded in base64" << std::setw(40)
              << Helper::getEnvVariable("AUTH_REASONER_SERVER_BASE64",
                                        DEFAULT_AUTH_REASONER_SERVER_BASE64)
              << "\n";

    std::cout << std::left << std::setw(35) << "REASONER_DATASTORE_NAME" << std::setw(65)
              << "Datastore name of the reasoner server" << std::setw(40)
              << Helper::getEnvVariable("REASONER_DATASTORE_NAME", DEFAULT_REASONER_DATASTORE_NAME)
              << "\n";
}

void displayHelpXOptions() {
    std::string bold = "\033[1m";
    std::string light_yellow = "\033[1;33m";
    std::string reset = "\033[0m";

    std::cout << bold << "Implementation-specific Options:\n" << reset;

    std::cout << "The following options are available:\n";

    std::cout << bold << std::left << std::setw(35) << "-X reset_ds" << std::setw(65)
              << ": reset the reasoner datastore." << reset << "\n";
}

int main(int argc, char* argv[]) {
    // Print the banner
    printBanner();

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--help") {
            displayHelp();
            return EXIT_SUCCESS;
        } else if (arg == "--help-env") {
            displayEnvVariables();
            return EXIT_SUCCESS;
        } else if (arg == "--help-xoptions") {
            displayHelpXOptions();
            return EXIT_SUCCESS;
        } else if (arg == "-X") {
            // Handle implementation-specific options here
            if (i + 1 < argc) {
                std::string option = argv[++i];
                if (option == "reset_ds") {
                    RESET_REASONER_DATASTORE = true;
                } else {
                    std::cerr << "Unknown implementation-specific option: " << option << "\n";
                    std::cerr << "Use --help-xoptions for available options.\n";
                    return EXIT_FAILURE;
                }
            } else {
                std::cerr << "Error: -X option requires an argument.\n";
                return EXIT_FAILURE;
            }
        } else {
            std::cerr << "Unknown argument: " << arg << "\n";
            std::cerr << "Use --help for usage information.\n";
            return EXIT_FAILURE;
        }
    }

    try {
        // Initialize System Configuration
        SystemConfig system_config = SystemConfigurationService::loadSystemConfig(
            DEFAULT_HOST_WEB_SOCKET_SERVER, DEFAULT_PORT_WEB_SOCKET_SERVER,
            DEFAULT_TARGET_WEB_SOCKET_SERVER, DEFAULT_REASONER_SERVER, DEFAULT_PORT_REASONER_SERVER,
            DEFAULT_AUTH_REASONER_SERVER_BASE64, DEFAULT_REASONER_DATASTORE_NAME);

        // Initialize Model Configuration
        std::shared_ptr<ModelConfig> model_config = std::make_shared<ModelConfig>(
            SystemConfigurationService::loadModelConfig(MODEL_CONFIGURATION_FILE));

        // Initialize Reasoner Service
        std::shared_ptr<ReasonerService> reasoner_service = ReasonerFactory::initReasoner(
            model_config->getReasonerSettings().getInferenceEngine(), system_config.reasoner_server,
            model_config->getReasonerRules(), model_config->getOntologies(),
            RESET_REASONER_DATASTORE);

        // Create the WebSocketClient
        std::cout << std::endl << "** Starting Websocket Client **" << std::endl;
        auto client =
            std::make_shared<WebSocketClient>(system_config, model_config, reasoner_service);

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