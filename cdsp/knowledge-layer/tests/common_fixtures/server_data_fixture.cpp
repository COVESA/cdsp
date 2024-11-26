#include "server_data_fixture.h"

ServerData ServerDataFixture::getValidWebsocketServerData() {
    // Set default values for the fixture
    ServerData data;
    data.host = "127.0.0.1";
    data.port = "8080";
    data.auth_base64 = "";
    data.data_store = std::nullopt;
    return data;
}

ServerData ServerDataFixture::getValidRDFoxServerData() {
    // Set default values for the fixture
    ServerData data;
    data.host = "127.0.0.1";
    data.port = "12110";
    data.auth_base64 = "cm9vdDphZG1pbg==";  // Base64 encoded authorization string
    data.data_store = "test_ds";
    return data;
}
