#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

#include "data_types.h"
#include "websocket_client.h"

using json = nlohmann::json;

boost::uuids::random_generator uuidGenerator;

std::string DefaultHostWebsocketServer{"127.0.0.1"};
std::string DefaultPortWebSocketServer{"8080"};
std::string DefaultRequiredVSSDataPointsFile{"vss_data_required.txt"};

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
    std::cout << bold << std::left << std::setw(50) << "Variable" << std::setw(50) << "Description"
              << "Value" << reset << "\n";
    std::cout << std::string(140, '-') << "\n";  // Line separator

    std::cout << std::left << std::setw(50) << "HOST_WEBSOCKET_SERVER" << std::setw(50)
              << "IP address of the WebSocket server"
              << getEnvVariable(" - HOST_WEBSOCKET_SERVER", DefaultHostWebsocketServer) << "\n";
    std::cout << std::left << std::setw(50) << "PORT_WEBSOCKET_SERVER" << std::setw(50)
              << "Port number of the WebSocket server"
              << getEnvVariable(" - PORT_WEBSOCKET_SERVER", DefaultPortWebSocketServer) << "\n";
    std::cout << std::left << std::setw(50) << "VIN" << std::setw(50)
              << "Object ID to be used in communication"
              << getEnvVariable("VIN", lightRed + "`Not Set (Required)`" + reset) << "\n";
    std::cout << std::left << std::setw(50) << "REQUIRED_VSS_DATA_POINTS_FILE" << std::setw(50)
              << "Path to the model configuration file"
              << getEnvVariable(" - REQUIRED_VSS_DATA_POINTS_FILE",
                                DefaultRequiredVSSDataPointsFile)
              << "\n";

    std::cout << "\n" << bold << "Description:\n" << reset;
    std::cout << "This client connects to a WebSocket server and processes incoming messages based "
                 "on the defined input.\n";
    std::cout << "The above environment variables are used to configure the application.\n\n";
}

/**
 * @brief Reads a file and returns a list of required data points.
 *
 * This function constructs the full path to the file using the provided file name
 * and a predefined project root directory. It then reads the file line by line,
 * storing each line as an element in a vector of strings.
 *
 * @param file_name The name of the file to read.
 * @return A vector of strings, each representing a required data point from the file.
 * @throws std::runtime_error if the file cannot be opened.
 */
std::vector<std::string> getClientRequiredDataPoints(std::string file_name) {
    std::vector<std::string> required_data;
    std::string root =
        std::string(PROJECT_ROOT) + "/symbolic-reasoner/examples/usecase-model/inputs/" + file_name;
    std::ifstream file(root);
    if (!file) {
        throw std::runtime_error("Invalid required Data Points file: " + file_name);
    }
    std::string line;
    while (std::getline(file, line)) {
        required_data.push_back(line);
    }
    return required_data;
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
    InitConfig init_config;
    init_config.host_websocket_server =
        getEnvVariable("HOST_WEBSOCKET_SERVER", DefaultHostWebsocketServer);
    init_config.port_websocket_server =
        getEnvVariable("PORT_WEBSOCKET_SERVER", DefaultPortWebSocketServer);
    init_config.uuid = boost::uuids::to_string(uuidGenerator());
    init_config.vin = getEnvVariable("VIN");
    init_config.system_vss_data_points = getClientRequiredDataPoints(
        getEnvVariable("REQUIRED_VSS_DATA_POINTS_FILE", DefaultRequiredVSSDataPointsFile));
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
