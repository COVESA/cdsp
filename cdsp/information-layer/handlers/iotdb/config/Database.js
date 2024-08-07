/*
 * Contains the definition of the database name and its identifier endpoint for each catalog.
 */
const Database = Object.freeze({
  VSS: {
    database_name: "root.Vehicles", // name of the configured IoTDB for the VSS Database
    endpoint_id: "VehicleIdentification_VIN", // endpoint used as element ID
  },
});

module.exports = Database;
