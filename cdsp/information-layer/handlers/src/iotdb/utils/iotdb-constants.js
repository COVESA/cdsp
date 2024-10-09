const IoTDBDataType = Object.freeze({
  BOOLEAN: 0,
  INT32: 1,
  INT64: 2,
  FLOAT: 3,
  DOUBLE: 4,
  TEXT: 5,
});

const SupportedMessageDataTypes = Object.freeze({
  boolean: "boolean",
  string: "string",
  float: "float",
  double: "double",
  int8: "int8",
  int16: "int16",
  uint8: "uint8",
  uint16: "uint16",
});

module.exports = { IoTDBDataType, SupportedMessageDataTypes };
