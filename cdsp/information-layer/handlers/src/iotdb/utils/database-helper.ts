import {SessionDataSet} from "./SessionDataSet";
import {IoTDBDataInterpreter} from "./IoTDBDataInterpreter";
import {removeSuffixFromString, replaceUnderscoresWithDots} from "../../../utils/transformations";


const METADATA_SUFFIX_IOTDB = ".Metadata";

// Valid data point is of type number or string
function isValidDatapoint(value: any) {
  if (value === null || value === undefined) return false;

  if (typeof value === 'number') {
    return !isNaN(value);
  }

  if (typeof value === 'string') {
    return true;
  }

  return false;
}

// Valid metadata is of type object 
function isValidMetadata(value: any) {
  return value !== null && typeof value === "object"
}

const removeNullFields = (obj: Record<string, any>): Record<string, any> => {
  return Object.fromEntries(
    Object.entries(obj).filter(([_, value]) => value !== null)
  );
};

function convertToMetadataTimestamp(timestampInMilliseconds: number) {
  const seconds = Math.floor(timestampInMilliseconds / 1000); // Convert milliseconds to seconds
  const nanos = (timestampInMilliseconds % 1000) * 1e6; // Remainder in milliseconds converted to nanoseconds
  const metadataTimestamp =
    {
      timestamps: {
        received: {
          seconds: seconds,
          nanos: nanos
        }
      }
    }
  return metadataTimestamp;
}

function deepMerge(target: Record<string, any>, source: Record<string, any>): Record<string, any> {
  for (const key in source) {
    if (source[key] && typeof source[key] === "object" && !Array.isArray(source[key])) {
      // If the key exists and is an object, merge deeply
      target[key] = deepMerge(target[key] || {}, source[key]);
    } else {
      // Otherwise, assign the new value (primitives will overwrite)
      target[key] = source[key];
    }
  }
  return target;
}

function insertMetadataReceivedTimestamp(transformedObject: Record<string, any>, timestampMilliseconds: number) {
  const metadataTimestamp = convertToMetadataTimestamp(timestampMilliseconds);
  let dataPointsNames = Object.keys(transformedObject).filter(value => !value.endsWith(METADATA_SUFFIX_IOTDB));

  dataPointsNames.forEach(dataPoints => {
    let metadataForDataPoints = transformedObject[dataPoints + METADATA_SUFFIX_IOTDB];
    // create new metadata with received timestamp
    if (!metadataForDataPoints) {
      transformedObject[dataPoints + METADATA_SUFFIX_IOTDB] = metadataTimestamp
    } else { // append received timestamp to existing metadata
      transformedObject[dataPoints + METADATA_SUFFIX_IOTDB] = deepMerge(JSON.parse(metadataForDataPoints), metadataTimestamp);
    }
  })

  return transformedObject;
}

/**
 * Transforms a session data set to 2 sets with the latest values of a vehicle, one for data points and one for metadata.
 * Metadata is only returned if it is available.
 *
 * @param sessionDataSet the session data set to be transformed
 * @param databaseName database name to be replaced during transformation
 * @returns 2 sets of latest values of a vehicles data points and relative metadata
 */
export function transformSessionDataSet(
  sessionDataSet: SessionDataSet,
  databaseName: string
): [Array<{ name: string; value: any }>, Array<{ name: string; value: any }>] {

  const mediaElements: any[] = [];
  while (sessionDataSet.hasNext()) {
    mediaElements.push(sessionDataSet.next());
  }

  const latestDataPoints: Record<string, any> = {};
  const latestMetadata: Record<string, any> = {};
  mediaElements.forEach((mediaElement) => {
    // filter out null values
    const existingElements = removeNullFields(mediaElement);
    const transformedMediaElement = Object.fromEntries(
      Object.entries(existingElements).map(([key, value]) => {
        const newKey = replaceUnderscoresWithDots(key);
        return [newKey, value];
      })
    );

    const transformedObject = IoTDBDataInterpreter.extractNodesFromTimeseries(
      transformedMediaElement,
      databaseName
    );

    const valuesAndMetadata = insertMetadataReceivedTimestamp(transformedObject, Number(transformedMediaElement.timestamp.timestamp))

    Object.entries(valuesAndMetadata).forEach(([key, value]) => {
      if (key.endsWith(METADATA_SUFFIX_IOTDB) && isValidMetadata(value)) {
        latestMetadata[removeSuffixFromString(key, METADATA_SUFFIX_IOTDB)] = value;
      } else if (isValidDatapoint(value)) {
        latestDataPoints[key] = value;
      }
    });
  });

  const mapToNameValue = ([name, value]: [string, any]) => ({name, value});
  const dataPoints = Object.entries(latestDataPoints).map(mapToNameValue);
  const metadata = Object.entries(latestMetadata).map(mapToNameValue);
  return [dataPoints, metadata];
}
