#include <gtest/gtest.h>

#include "file_handler_impl.h"
#include "globals.h"
#include "model_config.h"
#include "reasoner_factory.h"
#include "reasoner_service.h"
#include "reasoning_query_service.h"
#include "server_data_fixture.h"
#include "system_configuration_service.h"
#include "vin_utils.h"

class ReasoningQueryServiceTest : public ::testing::Test {
   protected:
    // NOLINTBEGIN(cppcoreguidelines-non-private-member-variables-in-classes)
    std::shared_ptr<FileHandlerImpl> file_handler_;
    const ReasonerServerData server_data_ = ServerDataFixture::getValidRDFoxServerData();
    std::shared_ptr<ReasonerService> reasoner_service_;
    // NOLINTEND(cppcoreguidelines-non-private-member-variables-in-classes)

    void SetUp() override {
        file_handler_ = std::make_shared<FileHandlerImpl>();
        setenv("VEHICLE_OBJECT_ID", VinUtils::getRandomVinString().c_str(), 1);
    }

    void TearDown() override {
        // Clean up
        if (reasoner_service_) {
            reasoner_service_->deleteDataStore();
        }
    }
};

/**
 * @brief Tests the processing of a regular reasoning query.
 *
 * This test initializes the ReasonerService with a model configuration that
 * does not use delta queries. It loads a set of triples into the reasoner and
 * then processes the reasoning queries defined in the model configuration.
 * The test verifies that the results match the expected output.
 */
TEST_F(ReasoningQueryServiceTest, ProcessRegularReasoningQuery_Success) {
    // Arrange
    setPathToUseCases(
        "/common-test-resources/common_fixtures/"
        "use_case_fixture/model/");

    // Initialize System Configuration
    SystemConfig system_config;
    system_config.reasoner_server = ServerDataFixture::getValidRDFoxServerData();

    const auto model_config =
        std::make_shared<ModelConfig>(SystemConfigurationService::loadModelConfig(
            getProjectRoot() + getPathToUseCases() + "model_config.json"));

    reasoner_service_ = ReasonerFactory::initReasoner(
        model_config->getReasonerSettings().getInferenceEngine(), system_config.reasoner_server,
        model_config->getReasonerRules(), model_config->getOntologies(), false);

    if (!reasoner_service_) {
        throw std::runtime_error("Failed to initialize the reasoner service.");
    }

    ReasoningQueryService reasoning_query_service(reasoner_service_);

    // ** Act 1 **
    // Load data and perform reasoning query

    std::vector<nlohmann::json> expected_results;
    expected_results.push_back(R"(    
      {"Vehicle":
        {"AI.Reasoner.InferenceResults":"{\"Chassis.SteeringWheel.Angle\":55.0,\"Speed\":100.0}"}
      }
  )"_json);

    const std::string triple_data = R"(
    @prefix bmw: <http://groupontology.bmwgroup.net/bmw-ont#> .
    @prefix sosa: <http://www.w3.org/ns/sosa/> .
    @prefix xsd: <http://www.w3.org/2001/XMLSchema#> .

    bmw:VehicleObservation_speed_20251021114517921301000
      a sosa:Observation ;
      sosa:hasFeatureOfInterest bmw:VehicleVINABC_Vehicle ;
      sosa:hasSimpleResult "100"^^xsd:float ;
      sosa:observedProperty bmw:speed ;
      sosa:phenomenonTime "2025-10-21T11:45:17.921301000Z"^^xsd:dateTime .

    bmw:VehicleVINABC_Vehicle
      a bmw:Vehicle .

    bmw:ChassisVINABC_Vehicle
      a bmw:Chassis ;
      bmw:hasPart bmw:SteeringWheelVINABC_Vehicle .

    bmw:SteeringWheelVINABC_Vehicle
      a bmw:SteeringWheel .

    bmw:VehicleObservation_angle_20251021114518921301000
      a sosa:Observation ;
      sosa:hasFeatureOfInterest bmw:SteeringWheelVINABC_Vehicle ;
      sosa:hasSimpleResult "55"^^xsd:float ;
      sosa:observedProperty bmw:angle ;
      sosa:phenomenonTime "2025-10-21T11:45:18.921301000Z"^^xsd:dateTime .

    bmw:VehicleVINABC_Vehicle
      a bmw:Vehicle ;
      bmw:hasPart bmw:ChassisVINABC_Vehicle .
  )";

    reasoner_service_->loadData(triple_data, model_config->getReasonerSettings().getOutputFormat());

    auto results_1 = reasoning_query_service.processReasoningQuery(
        model_config->getReasoningOutputQueries().at(0),
        model_config->getReasonerSettings().isIsAiReasonerInferenceResults(), std::nullopt);

    // Assert: Expect that results contain the new delta changes
    ASSERT_EQ(results_1.size(), expected_results.size());
    for (size_t i = 0; i < results_1.size(); ++i) {
        EXPECT_EQ(results_1[i], expected_results[i]);
    }

    // ** Act 2 **
    // Perform reasoning query again returning same results

    auto results_2 = reasoning_query_service.processReasoningQuery(
        model_config->getReasoningOutputQueries().at(0),
        model_config->getReasonerSettings().isIsAiReasonerInferenceResults(), std::nullopt);

    // Assert: Expect that results contain the new delta changes
    ASSERT_EQ(results_2.size(), expected_results.size());
    for (size_t i = 0; i < results_2.size(); ++i) {
        EXPECT_EQ(results_2[i], expected_results[i]);
    }
}