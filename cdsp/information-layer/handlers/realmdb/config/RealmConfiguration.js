const Database = require("./Database");

const MediaElementSchema = {
  primaryKey: "_id",
  name: Database.VSS.database_name,
  properties: {
    _id: "string",
    VehicleIdentification_VIN: "string",
    CurrentLocation_Latitude: "double",
    CurrentLocation_Longitude: "double",
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
