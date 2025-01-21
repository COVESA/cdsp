#include "init_config_fixture.h"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "model_config_fixture.h"
#include "server_data_fixture.h"

InitConfig InitConfigFixture::getValidInitConfig(const std::string& oid) {
    InitConfig config;
    config.uuid = generateUUID();
    config.websocket_server = ServerDataFixture::getValidWebsocketServerData();
    config.rdfox_server = ServerDataFixture::getValidRDFoxServerData();
    config.oid = oid;
    config.model_config = ModelConfigFixture::getValidModelConfig();
    return config;
}

// Create a random UUID generator
std::string InitConfigFixture::generateUUID() {
    boost::uuids::random_generator generator;
    boost::uuids::uuid uuid = generator();
    return boost::uuids::to_string(uuid);
}