/*
 * Contains the definition of the database name and its identifier endpoint for each catalog.
 */
const databaseParams = Object.freeze({
  VSS: {
    databaseName: "Vehicles", // name of the configured RealmDB for the VSS database
    endpointId: "Vehicle_VehicleIdentification_VIN", // endpoint used as element ID
  },
});

module.exports = databaseParams;
