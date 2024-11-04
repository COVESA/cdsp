import { SessionDataSet } from "./SessionDataSet";
import { IoTDBDataInterpreter } from "./IoTDBDataInterpreter";

/**
 * Transforms a session data set to a set with the latest values of a vehicle.
 * 
 * @param sessionDataSet the session data set to be transformed
 * @param databaseName database name to be replaced during transformation
 * @returns set of latest values of a vehicle
 */
export function transformSessionDataSet(
    sessionDataSet: SessionDataSet,
    databaseName: string
  ): Array<{ name: string; value: any }> {
    const mediaElements: any[] = [];
    while (sessionDataSet.hasNext()) {
      mediaElements.push(sessionDataSet.next());
    }

    const latestValues: Record<string, any> = {};
    mediaElements.forEach((mediaElement) => {
      const transformedMediaElement = Object.fromEntries(
        Object.entries(mediaElement).map(([key, value]) => {
          const newKey = transformDataPointsWithDots(key);
          return [newKey, value];
        })
      );

      const transformedObject = IoTDBDataInterpreter.extractNodesFromTimeseries(
        transformedMediaElement,
        databaseName
      );

      Object.entries(transformedObject).forEach(([key, value]) => {
        if (value !== null && !isNaN(value)) {
          latestValues[key] = value;
        }
      });
    });

    return Object.entries(latestValues).map(([name, value]) => ({
        name,
        value,
      }));
  }

  /**
   * Transforms a database field name by replacing underscores with dots.
   * @param field - The database filed to transform.
   * @returns - The transformed to message node replacing underscores by dots.
   */
  function transformDataPointsWithDots(field: string): string {
    return field.replace(/\_/g, ".");
  }
