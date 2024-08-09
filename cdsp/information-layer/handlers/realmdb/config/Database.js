/*
 * Contains the definition of the database name and its identifier endpoint for each catalog.
 */
const Database = Object.freeze({
  VSS: {
    database_name: "Vehicles", // name of the configured RealmDB for the VSS Database
    endpoint_id: "Vehicle_VehicleIdentification_VIN", // endpoint used as element ID
  },
});

module.exports = Database;
