// Define IoTDBDataType as an enum for better type safety
export enum IoTDBDataType {
  BOOLEAN = 0,
  INT32 = 1,
  INT64 = 2,
  FLOAT = 3,
  DOUBLE = 4,
  TEXT = 5,
}

// Define SupportedMessageDataTypes as a frozen object with a string-to-string mapping
export const SupportedMessageDataTypes = {
  boolean: "boolean",
  string: "string",
  float: "float",
  double: "double",
  int8: "int8",
  int16: "int16",
  uint8: "uint8",
  uint16: "uint16",
} as const;

export const METADATA_SUFFIX = "_Metadata";
