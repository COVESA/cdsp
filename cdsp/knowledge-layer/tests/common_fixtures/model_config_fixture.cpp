#include "model_config_fixture.h"

ModelConfig ModelConfigFixture::getValidModelConfig() {
    // Set default values for the fixture
    ReasonerSettings settings;
    settings.inference_engine = "RDFox";
    settings.output_format = RDFSyntaxType::TURTLE;
    settings.supported_tree_types = {"vss"};

    ModelConfig config;
    config.system_data_points = {
        {"vss",
         {"Vehicle.Chassis.SteeringWheel.Angle", "Vehicle.CurrentLocation.Latitude",
          "Vehicle.CurrentLocation.Longitude", "Vehicle.Powertrain.TractionBattery.NominalVoltage",
          "Vehicle.Powertrain.TractionBattery.StateOfCharge.CurrentEnergy",
          "Vehicle.Powertrain.Transmission.CurrentGear", "Vehicle.Speed"}}};

    config.output_file_path = "output/";
    config.ontology_files = {"ontologies/example_ontology.ttl"};
    config.shacl_shapes_files = {"shacl/vss_shacl.ttl", "shacl/observation_shacl.ttl"};
    config.triple_assembler_queries_files = {
        {"default", {}},
        {"vss",
         {"queries/triple_assembler_helper/vss/data_property.rq",
          "queries/triple_assembler_helper/vss/object_property.rq"}}};

    config.output_queries_path = "queries/output/";
    config.rules_files = {"rules/insight_rules.ttl"};
    config.reasoner_settings = settings;

    return config;
}