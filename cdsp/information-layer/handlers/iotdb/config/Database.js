/*
 * Contains the definition of the database name and its identifier endpoint for each catalog.
 */
const database = Object.freeze({
  VSS: {
    database_name: "root.Vehicles", // name of the configured IoTDB for the VSS database
    endpoint_id: "Vehicle_VehicleIdentification_VIN", // endpoint used as element ID
  },
});

module.exports = database;
