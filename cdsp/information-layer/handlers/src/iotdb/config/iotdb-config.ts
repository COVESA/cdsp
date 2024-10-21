import { SupportedMessageDataTypes } from "../utils/iotdb-constants";

export interface SupportedDataPoints {
  [key: string]: keyof typeof SupportedMessageDataTypes;
}

/**
 * Validates and creates the data points schema.
 * Ensures only supported data types are included.
 *
 * @param supportedDataPoints - An object containing the supported data points.
 * @returns The validated and immutable data points schema.
 */
function buildSchema(
  supportedDataPoints: SupportedDataPoints,
): SupportedDataPoints {
  const properties: SupportedDataPoints = {};

  Object.entries(supportedDataPoints).forEach(([key, value]) => {
    if (isSupportedDataPoint(value)) {
      properties[key] = value;
    } else {
      throw new Error(
        `The initialized data points contains an unsupported data type: ${value}`,
      );
    }
  });

  return Object.freeze(properties);
}
/**
 * Type guard to check if a data type is supported by SupportedMessageDataTypes.
 *
 * @param type - The data type to check.
 * @returns A boolean indicating whether the type is valid.
 */
export function isSupportedDataPoint(
  type: string,
): type is keyof typeof SupportedMessageDataTypes {
  return type in SupportedMessageDataTypes;
}

// Singleton instance holder
let dataPointsSchemaInstance: SupportedDataPoints | null = null;

/**
 * Creates and returns a singleton instance of DataPointsSchema.
 * If the instance does not already exist, it initializes it with the provided supported data points
 * and freezes the instance to prevent further modifications.
 *
 * @param supportedEndpoints - An object of supported data points to initialize the schema.
 * @returns The data points schema instance.
 */
export function createDataPointsSchema(
  supportedEndpoints: SupportedDataPoints,
): SupportedDataPoints {
  if (!dataPointsSchemaInstance) {
    dataPointsSchemaInstance = buildSchema(supportedEndpoints);
  }

  return dataPointsSchemaInstance;
}
