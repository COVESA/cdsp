const { MessageDataType } = require("../utils/IoTDBConstants");

/*
 * Define the supported endpoints and the corresponding data types.
 */
const endpointsType = Object.freeze({
  Vehicle_Chassis_SteeringWheel_Angle: MessageDataType.INT16,
  Vehicle_CurrentLocation_Latitude: MessageDataType.DOUBLE,
  Vehicle_CurrentLocation_Longitude: MessageDataType.DOUBLE,
  Vehicle_Powertrain_TractionBattery_NominalVoltage: MessageDataType.UINT16,
  Vehicle_Powertrain_TractionBattery_StateOfCharge_CurrentEnergy:
    MessageDataType.FLOAT,
  Vehicle_Powertrain_Transmission_CurrentGear: MessageDataType.INT8,
  Vehicle_Speed: MessageDataType.FLOAT,
  Vehicle_VehicleIdentification_VIN: MessageDataType.STRING,
});

module.exports = endpointsType;
