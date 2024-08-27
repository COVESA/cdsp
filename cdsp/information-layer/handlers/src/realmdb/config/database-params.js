/*
 * Contains the definition of the database name and its identifier endpoint for each catalog.
 */
const mediaElementsParams = Object.freeze({
  VSS: {
    databaseName: "Vehicles", // name of the configured RealmDB for the VSS database
    endpointId: "Vehicle_VehicleIdentification_VIN", // endpoint used as element ID
  },
});

const databaseConfig = Object.freeze({
  storePath: "myrealm12.realm",
});

module.exports = { mediaElementsParams, databaseConfig };
