const IoTDBDataType = Object.freeze({
  BOOLEAN: 0,
  INT32: 1,
  INT64: 2,
  FLOAT: 3,
  DOUBLE: 4,
  TEXT: 5,
});

const MessageDataType = Object.freeze({
  INT8: 0,
  INT16: 1,
  INT32: 2,
  UINT16: 3,
  FLOAT: 4,
  DOUBLE: 5,
  STRING: 6,
});

module.exports = { IoTDBDataType, MessageDataType };
