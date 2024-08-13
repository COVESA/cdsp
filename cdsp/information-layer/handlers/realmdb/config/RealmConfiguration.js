const database = require("./databaseParams");

const MediaElementSchema = {
  primaryKey: "_id",
  name: database.VSS.databaseName,
  properties: {
    _id: "string",
    Vehicle_Chassis_SteeringWheel_Angle: "int",
    Vehicle_CurrentLocation_Latitude: "double",
    Vehicle_CurrentLocation_Longitude: "double",
    Vehicle_Powertrain_TractionBattery_NominalVoltage: "int",
    Vehicle_Powertrain_TractionBattery_StateOfCharge_CurrentEnergy: "float",
    Vehicle_Powertrain_Transmission_CurrentGear: "int",
    Vehicle_Speed: "float",
    Vehicle_VehicleIdentification_VIN: "string",
  },
};

/**
 * Generates a configuration object for a Realm database.
 *
 * @param {object} user - The user object from the authentication.
 * @returns {object} The configuration object for the Realm database.
 */
const realmConfig = (user) => ({
  schema: [MediaElementSchema],
  path: "myrealm12.realm",
  sync: {
    user: user,
    flexible: true,
    error: (error) => {
      console.error("Realm sync error:", error);
    },
  },
});

module.exports = realmConfig;
