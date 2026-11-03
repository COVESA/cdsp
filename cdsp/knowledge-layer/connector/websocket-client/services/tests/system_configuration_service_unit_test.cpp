#include <gtest/gtest.h>

#include <string>

#include "globals.h"
#include "system_configuration_service.h"

TEST(SystemConfigurationServiceUnitTest, DescribeSystemConfigShowsSettingsButNotCredentials) {
    SystemConfig system_config;
    system_config.websocket_server = {"information-layer", "8080", ""};
    system_config.reasoner_server = {"rdf4j-stream-reasoner-service", "12110", "c2VjcmV0",
                                     "SemanticReasoner", "ds-test"};
    setPathToUseCases("/symbolic-reasoner/examples/use-case/rdf4j_model/");

    const std::string summary = SystemConfigurationService::describeSystemConfig(system_config);

    EXPECT_NE(summary.find("information-layer:8080"), std::string::npos);
    EXPECT_NE(summary.find("rdf4j-stream-reasoner-service:12110"), std::string::npos);
    EXPECT_NE(summary.find("Reasoner credentials: set"), std::string::npos);
    EXPECT_NE(summary.find("ds-test"), std::string::npos);
    EXPECT_NE(summary.find("use-case/rdf4j_model/"), std::string::npos);
    EXPECT_EQ(summary.find("c2VjcmV0"), std::string::npos);
}
