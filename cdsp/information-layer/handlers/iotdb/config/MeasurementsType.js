const JSDataType = require("../utils/IoTDBConstants");

const MeasurementType = Object.freeze({
  VIN: JSDataType.TEXT,
  Vehicle_Cabin_HVAC_AmbientAirTemperature: JSDataType.FLOAT,
});

module.exports = MeasurementType;
