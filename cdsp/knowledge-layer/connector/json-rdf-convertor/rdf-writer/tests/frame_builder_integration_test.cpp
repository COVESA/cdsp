#include <gtest/gtest.h>

#include <chrono>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "frame_builder.h"
#include "triple_writer.h"

using std::chrono::milliseconds;
using time_point = std::chrono::system_clock::time_point;

class FrameBuilderIntegrationTest : public ::testing::Test {
   protected:
    static constexpr milliseconds MAX_VALUE_AGE{1000};
    TripleWriter triple_writer_;
    FrameBuilder frame_builder_{triple_writer_, MAX_VALUE_AGE};

    const std::string VIN = "VIN123";
    const std::string PREFIXES =
        "prefix car: <http://example.ontology.com/car#>\n"
        "prefix xsd: <http://www.w3.org/2001/XMLSchema#>";
    // 2026-11-16T10:00:02Z
    const time_point T0 = time_point(std::chrono::seconds(1794823202));

    static std::string car(const std::string& name) {
        return "<http://example.ontology.com/car#" + name + ">";
    }
    static std::string xsd(const std::string& name) {
        return "<http://www.w3.org/2001/XMLSchema#" + name + ">";
    }

    ResolvedSignal angle(const std::string& value, const time_point& timestamp) const {
        return ResolvedSignal{
            {{PREFIXES, {car("Vehicle"), car("hasChassis"), car("Chassis")}},
             {PREFIXES, {car("Chassis"), car("hasSteeringWheel"), car("SteeringWheel")}}},
            {PREFIXES, {car("SteeringWheel"), car("Angle"), xsd("short")}},
            value,
            timestamp};
    }

    ResolvedSignal speed(const std::string& value, const time_point& timestamp) const {
        return ResolvedSignal{
            {}, {PREFIXES, {car("Vehicle"), car("Speed"), xsd("float")}}, value, timestamp};
    }

    ResolvedSignal coordinate(const std::string& name, const std::string& value,
                              const time_point& timestamp) const {
        return ResolvedSignal{
            {{PREFIXES, {car("Vehicle"), car("hasCurrentLocation"), car("CurrentLocation")}}},
            {PREFIXES, {car("CurrentLocation"), car(name), xsd("double")}},
            value,
            timestamp};
    }

    std::optional<BuiltFrame> addMessage(
        const std::string& identifier,
        const std::vector<std::pair<std::string, ResolvedSignal>>& signals) {
        return frame_builder_.addMessage(identifier, signals, ReasonerSyntaxType::TURTLE);
    }

    std::string build(const time_point& frame_time) {
        return frame_builder_.buildFrame(VIN, frame_time, ReasonerSyntaxType::TURTLE);
    }

    static bool contains(const std::string& text, const std::string& part) {
        return text.find(part) != std::string::npos;
    }
};

/**
 * @brief A frame with angle and speed: class triples, one member per signal with its own time,
 * and the frame node with the frame time and the value of each member.
 */
TEST_F(FrameBuilderIntegrationTest, BuildsFrameWithOneMemberPerSignal) {
    frame_builder_.updateSignal(VIN, "Vehicle.Chassis.SteeringWheel.Angle",
                                angle("250", T0 + milliseconds(200)));
    frame_builder_.updateSignal(VIN, "Vehicle.Speed", speed("64.0", T0 + milliseconds(187)));

    const std::string expected = R"(@prefix car: <http://example.ontology.com/car#> .
@prefix sosa: <http://www.w3.org/ns/sosa/> .
@prefix xsd: <http://www.w3.org/2001/XMLSchema#> .

car:VehicleVIN123
	a car:Vehicle ;
	car:hasChassis car:ChassisVIN123 .

car:ChassisVIN123
	a car:Chassis ;
	car:hasSteeringWheel car:SteeringWheelVIN123 .

car:SteeringWheelVIN123
	a car:SteeringWheel .

car:fm_Angle_VIN123_20261116100002200000000
	a sosa:Observation ;
	sosa:hasFeatureOfInterest car:SteeringWheelVIN123 ;
	sosa:hasSimpleResult "250"^^xsd:short ;
	sosa:observedProperty car:Angle ;
	sosa:phenomenonTime "2026-11-16T10:00:02.200000000Z"^^xsd:dateTime .

car:VehicleVIN123
	a car:Vehicle .

car:fm_Speed_VIN123_20261116100002200000000
	a sosa:Observation ;
	sosa:hasFeatureOfInterest car:VehicleVIN123 ;
	sosa:hasSimpleResult "64.0"^^xsd:float ;
	sosa:observedProperty car:Speed ;
	sosa:phenomenonTime "2026-11-16T10:00:02.187000000Z"^^xsd:dateTime .

car:frame_VIN123_20261116100002200000000
	a sosa:ObservationCollection ;
	sosa:hasFeatureOfInterest car:VehicleVIN123 ;
	sosa:phenomenonTime "2026-11-16T10:00:02.200000000Z"^^xsd:dateTime ;
	car:Angle "250"^^xsd:short ;
	car:Speed "64.0"^^xsd:float ;
	sosa:hasMember car:fm_Angle_VIN123_20261116100002200000000 ,
		car:fm_Speed_VIN123_20261116100002200000000 .)";

    EXPECT_EQ(build(T0 + milliseconds(200)), expected);
}

/**
 * @brief A newer value replaces the kept one, an older value is ignored.
 */
TEST_F(FrameBuilderIntegrationTest, KeepsLatestValuePerSignal) {
    frame_builder_.updateSignal(VIN, "Vehicle.Speed", speed("10.0", T0));
    frame_builder_.updateSignal(VIN, "Vehicle.Speed", speed("20.0", T0 + milliseconds(100)));
    frame_builder_.updateSignal(VIN, "Vehicle.Speed", speed("30.0", T0 + milliseconds(50)));

    const std::string frame = build(T0 + milliseconds(100));
    EXPECT_TRUE(contains(frame, R"("20.0"^^xsd:float)")) << frame;
    EXPECT_FALSE(contains(frame, R"("10.0"^^xsd:float)")) << frame;
    EXPECT_FALSE(contains(frame, R"("30.0"^^xsd:float)")) << frame;
}

/**
 * @brief A value older than the maximum age before the frame time is left out; a value exactly
 * at the maximum age is kept.
 */
TEST_F(FrameBuilderIntegrationTest, LeavesOutValuesOlderThanMaxAge) {
    frame_builder_.updateSignal(VIN, "Vehicle.Speed", speed("64.0", T0));
    frame_builder_.updateSignal(VIN, "Vehicle.Chassis.SteeringWheel.Angle", angle("250", T0));

    const std::string at_max_age = build(T0 + MAX_VALUE_AGE);
    EXPECT_TRUE(contains(at_max_age, "car:fm_Speed_")) << at_max_age;

    frame_builder_.updateSignal(VIN, "Vehicle.Chassis.SteeringWheel.Angle",
                                angle("300", T0 + milliseconds(1500)));
    const std::string later = build(T0 + milliseconds(1500));
    EXPECT_FALSE(contains(later, "car:fm_Speed_")) << later;
    EXPECT_FALSE(contains(later, R"(car:Speed "64.0")")) << later;
    EXPECT_TRUE(contains(later, R"("300"^^xsd:short)")) << later;
}

/**
 * @brief Without a recent value there is no frame.
 */
TEST_F(FrameBuilderIntegrationTest, ReturnsEmptyWithoutRecentValues) {
    EXPECT_EQ(build(T0), "");

    frame_builder_.updateSignal(VIN, "Vehicle.Speed", speed("64.0", T0));
    EXPECT_EQ(build(T0 + milliseconds(2000)), "");
}

/**
 * @brief Latitude and longitude are only taken together, both recent enough.
 */
TEST_F(FrameBuilderIntegrationTest, TakesLocationOnlyAsPair) {
    frame_builder_.updateSignal(VIN, "Vehicle.Speed", speed("64.0", T0 + milliseconds(1500)));
    frame_builder_.updateSignal(VIN, "Vehicle.CurrentLocation.Latitude",
                                coordinate("Latitude", "48.1", T0 + milliseconds(1500)));

    std::string frame = build(T0 + milliseconds(1500));
    EXPECT_FALSE(contains(frame, "car:fm_Latitude_")) << frame;

    frame_builder_.updateSignal(VIN, "Vehicle.CurrentLocation.Longitude",
                                coordinate("Longitude", "11.5", T0 + milliseconds(1500)));
    frame = build(T0 + milliseconds(1500));
    EXPECT_TRUE(contains(frame, "car:fm_Latitude_")) << frame;
    EXPECT_TRUE(contains(frame, "car:fm_Longitude_")) << frame;
    EXPECT_TRUE(contains(frame, R"(car:Latitude "48.1"^^xsd:double)")) << frame;
    EXPECT_TRUE(contains(frame, R"(car:Longitude "11.5"^^xsd:double)")) << frame;

    // The longitude gets too old, so the latitude is left out as well
    frame_builder_.updateSignal(VIN, "Vehicle.CurrentLocation.Latitude",
                                coordinate("Latitude", "48.2", T0 + milliseconds(3000)));
    frame_builder_.updateSignal(VIN, "Vehicle.Speed", speed("65.0", T0 + milliseconds(3000)));
    frame = build(T0 + milliseconds(3000));
    EXPECT_FALSE(contains(frame, "car:fm_Latitude_")) << frame;
    EXPECT_FALSE(contains(frame, "car:fm_Longitude_")) << frame;
    EXPECT_FALSE(contains(frame, "car:Latitude ")) << frame;
}

/**
 * @brief The location pair gets its NTM values, like an observation without frames. The NTM
 * values stay on the members, the frame only carries the values.
 */
TEST_F(FrameBuilderIntegrationTest, AddsNtmValuesToLocationPair) {
    frame_builder_.updateSignal(VIN, "Vehicle.CurrentLocation.Latitude",
                                coordinate("latitude", "48.1", T0));
    frame_builder_.updateSignal(VIN, "Vehicle.CurrentLocation.Longitude",
                                coordinate("longitude", "11.5", T0));

    const std::string frame = build(T0);
    EXPECT_TRUE(contains(frame, "car:fm_latitude_")) << frame;
    EXPECT_TRUE(contains(frame, "car:fm_longitude_")) << frame;
    std::size_t ntm_values = 0;
    for (std::size_t pos = frame.find("car:hasSimpleResultNTM"); pos != std::string::npos;
         pos = frame.find("car:hasSimpleResultNTM", pos + 1)) {
        ++ntm_values;
    }
    EXPECT_EQ(ntm_values, 2) << frame;
}

/**
 * @brief A value repeated in the next frame gets a new member with the time of that frame.
 */
TEST_F(FrameBuilderIntegrationTest, RepeatedValueGetsNewMemberPerFrame) {
    frame_builder_.updateSignal(VIN, "Vehicle.Speed", speed("64.0", T0));

    const std::string first = build(T0);
    const std::string second = build(T0 + milliseconds(200));
    EXPECT_TRUE(contains(first, "car:fm_Speed_VIN123_20261116100002000000000")) << first;
    EXPECT_TRUE(contains(second, "car:fm_Speed_VIN123_20261116100002200000000")) << second;
    EXPECT_TRUE(contains(second, R"("2026-11-16T10:00:02.000000000Z"^^xsd:dateTime)")) << second;
}

/**
 * @brief Values are kept per vehicle: each frame only contains the values of its own vehicle.
 */
TEST_F(FrameBuilderIntegrationTest, KeepsValuesPerVehicle) {
    const std::string OTHER_VIN = "VIN456";
    frame_builder_.updateSignal(VIN, "Vehicle.Speed", speed("64.0", T0));
    frame_builder_.updateSignal(OTHER_VIN, "Vehicle.Speed", speed("90.0", T0 + milliseconds(100)));
    frame_builder_.updateSignal(OTHER_VIN, "Vehicle.Chassis.SteeringWheel.Angle",
                                angle("250", T0 + milliseconds(100)));

    const std::string frame = build(T0 + milliseconds(100));
    EXPECT_TRUE(contains(frame, R"("64.0"^^xsd:float)")) << frame;
    EXPECT_FALSE(contains(frame, R"("90.0"^^xsd:float)")) << frame;
    EXPECT_FALSE(contains(frame, "car:fm_Angle_")) << frame;
    EXPECT_FALSE(contains(frame, OTHER_VIN)) << frame;

    const std::string other_frame =
        frame_builder_.buildFrame(OTHER_VIN, T0 + milliseconds(100), ReasonerSyntaxType::TURTLE);
    EXPECT_TRUE(contains(other_frame, R"("90.0"^^xsd:float)")) << other_frame;
    EXPECT_TRUE(contains(other_frame, "car:fm_Angle_VIN456_")) << other_frame;
    EXPECT_FALSE(contains(other_frame, R"("64.0"^^xsd:float)")) << other_frame;
    EXPECT_FALSE(contains(other_frame, "VIN123")) << other_frame;

    // A vehicle without kept values gets no frame
    EXPECT_EQ(frame_builder_.buildFrame("VIN789", T0, ReasonerSyntaxType::TURTLE), "");
}

/**
 * @brief The signals of one moment arrive in several messages: the frame of that moment is built
 * once, when the first message of a later moment arrives, and does not contain the later values.
 */
TEST_F(FrameBuilderIntegrationTest, BuildsOneFramePerMoment) {
    EXPECT_FALSE(addMessage(VIN, {{"Vehicle.Chassis.SteeringWheel.Angle", angle("20", T0)}}));
    EXPECT_FALSE(addMessage(VIN, {{"Vehicle.Speed", speed("64.0", T0)}}));
    EXPECT_FALSE(addMessage(
        VIN, {{"Vehicle.CurrentLocation.Latitude", coordinate("Latitude", "48.1", T0)},
              {"Vehicle.CurrentLocation.Longitude", coordinate("Longitude", "11.5", T0)}}));

    const auto next_moment = T0 + milliseconds(200);
    const auto frame =
        addMessage(VIN, {{"Vehicle.Chassis.SteeringWheel.Angle", angle("250", next_moment)}});
    ASSERT_TRUE(frame.has_value());
    EXPECT_EQ(frame->time, T0);
    EXPECT_TRUE(contains(frame->triples, "car:frame_VIN123_20261116100002000000000"))
        << frame->triples;
    EXPECT_TRUE(contains(frame->triples, R"("20"^^xsd:short)")) << frame->triples;
    EXPECT_FALSE(contains(frame->triples, R"("250"^^xsd:short)")) << frame->triples;
    EXPECT_TRUE(contains(frame->triples, "car:fm_Speed_")) << frame->triples;
    EXPECT_TRUE(contains(frame->triples, "car:fm_Latitude_")) << frame->triples;
    EXPECT_TRUE(contains(frame->triples, "car:fm_Longitude_")) << frame->triples;
    EXPECT_FALSE(contains(frame->triples, "20261116100002200000000")) << frame->triples;

    // The next moment is open until a later message arrives
    EXPECT_FALSE(addMessage(VIN, {{"Vehicle.Speed", speed("70.0", next_moment)}}));
    const auto next_frame =
        addMessage(VIN, {{"Vehicle.Speed", speed("72.0", next_moment + milliseconds(200))}});
    ASSERT_TRUE(next_frame.has_value());
    EXPECT_EQ(next_frame->time, next_moment);
    EXPECT_TRUE(contains(next_frame->triples, R"("250"^^xsd:short)")) << next_frame->triples;
    EXPECT_TRUE(contains(next_frame->triples, R"("70.0"^^xsd:float)")) << next_frame->triples;
}

/**
 * @brief A message older than the open moment only updates the values; it builds no frame.
 */
TEST_F(FrameBuilderIntegrationTest, OlderMessageBuildsNoFrame) {
    EXPECT_FALSE(addMessage(VIN, {{"Vehicle.Speed", speed("64.0", T0 + milliseconds(200))}}));
    EXPECT_FALSE(addMessage(VIN, {{"Vehicle.Chassis.SteeringWheel.Angle", angle("20", T0)}}));

    const auto frame = addMessage(VIN, {{"Vehicle.Speed", speed("70.0", T0 + milliseconds(400))}});
    ASSERT_TRUE(frame.has_value());
    EXPECT_EQ(frame->time, T0 + milliseconds(200));
    EXPECT_TRUE(contains(frame->triples, R"("20"^^xsd:short)")) << frame->triples;
}

/**
 * @brief Moments are kept per vehicle: a message of another vehicle does not end a moment.
 */
TEST_F(FrameBuilderIntegrationTest, KeepsMomentsPerVehicle) {
    const std::string OTHER_VIN = "VIN456";
    EXPECT_FALSE(addMessage(VIN, {{"Vehicle.Speed", speed("64.0", T0)}}));
    EXPECT_FALSE(addMessage(OTHER_VIN, {{"Vehicle.Speed", speed("90.0", T0 + milliseconds(200))}}));

    const auto frame = addMessage(VIN, {{"Vehicle.Speed", speed("70.0", T0 + milliseconds(200))}});
    ASSERT_TRUE(frame.has_value());
    EXPECT_EQ(frame->time, T0);
    EXPECT_TRUE(contains(frame->triples, R"("64.0"^^xsd:float)")) << frame->triples;
    EXPECT_FALSE(contains(frame->triples, OTHER_VIN)) << frame->triples;
}

/**
 * @brief A message with an empty value is rejected before anything is kept.
 */
TEST_F(FrameBuilderIntegrationTest, AddMessageRejectsEmptyValue) {
    EXPECT_THROW(addMessage(VIN, {{"Vehicle.Speed", speed("", T0)}}), std::invalid_argument);
    EXPECT_EQ(build(T0), "");
}

/**
 * @brief An empty value is rejected.
 */
TEST_F(FrameBuilderIntegrationTest, RejectsEmptyValue) {
    EXPECT_THROW(frame_builder_.updateSignal(VIN, "Vehicle.Speed", speed("", T0)),
                 std::invalid_argument);
}

/**
 * @brief The maximum age comes from the constructor; a shorter age leaves out more values.
 */
TEST_F(FrameBuilderIntegrationTest, UsesConfiguredMaxAge) {
    TripleWriter triple_writer;
    FrameBuilder frame_builder(triple_writer, milliseconds(300));
    frame_builder.updateSignal(VIN, "Vehicle.Speed", speed("64.0", T0));
    frame_builder.updateSignal(VIN, "Vehicle.Chassis.SteeringWheel.Angle",
                               angle("250", T0 + milliseconds(500)));

    const std::string frame =
        frame_builder.buildFrame(VIN, T0 + milliseconds(500), ReasonerSyntaxType::TURTLE);
    EXPECT_FALSE(contains(frame, "car:fm_Speed_")) << frame;
    EXPECT_TRUE(contains(frame, "car:fm_Angle_")) << frame;
}

/**
 * @brief A maximum age that is not positive is rejected.
 */
TEST_F(FrameBuilderIntegrationTest, RejectsNonPositiveMaxAge) {
    TripleWriter triple_writer;
    EXPECT_THROW(FrameBuilder(triple_writer, milliseconds(0)), std::invalid_argument);
}
