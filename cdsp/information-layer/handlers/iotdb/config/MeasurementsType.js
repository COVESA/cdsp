const JSDataType = require("../utils/IoTDBConstants");

/*
 * Define the supported endpoints and the corresponding data types.
 */
const MeasurementType = Object.freeze({
  VehicleIdentification_VIN: JSDataType.TEXT,
  CurrentLocation_Latitude: JSDataType.DOUBLE,
  CurrentLocation_Longitude: JSDataType.DOUBLE,
});

module.exports = MeasurementType;
