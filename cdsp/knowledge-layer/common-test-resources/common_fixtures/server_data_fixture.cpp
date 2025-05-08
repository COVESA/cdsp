#include "server_data_fixture.h"

WSServerData ServerDataFixture::getValidWebsocketServerData() {
    // Set default values for the fixture
    WSServerData data;
    data.host = "127.0.0.1";
    data.port = "8080";
    data.target = "";
    return data;
}

ReasonerServerData ServerDataFixture::getValidRDFoxServerData() {
    // Set default values for the fixture
    ReasonerServerData data;
    data.host = "127.0.0.1";
    data.port = "12110";
    data.auth_base64 = "cm9vdDphZG1pbg==";  // Base64 encoded authorization string
    data.data_store_name = "test_ds";
    return data;
}
