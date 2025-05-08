import {Session} from "./Session";
import {getSubscriptionSimulator, SubscriptionSimulator} from "./SubscriptionSimulator";
import {SupportedMessageDataTypes, METADATA_SUFFIX} from "../utils/iotdb-constants";
import {HandlerBase, QueryResult} from "../../HandlerBase";
import {SessionDataSet} from "../utils/SessionDataSet";
import {IoTDBDataInterpreter} from "../utils/IoTDBDataInterpreter";
import {createDataPointsSchema, isSupportedDataPoint, SupportedDataPoints,} from "../config/iotdb-config";
import {databaseConfig, databaseParams} from "../config/database-params";
import {COLORS, logErrorStr, logWithColor,} from "../../../../utils/logger";
import {STATUS_ERRORS, STATUS_SUCCESS} from "../../../../router/utils/ErrorMessage";
import {WebSocketWithId} from "../../../../utils/database-params";
import {transformSessionDataSet} from "../utils/database-helper";
import {
  NewMessage,
  SetMessageType,
  StatusMessage,
  SubscribeMessageType, UnsubscribeMessageType
} from "../../../../router/utils/NewMessage";
import {removeSuffixFromString, replaceDotsWithUnderscore, replaceUnderscoresWithDots} from "../../../utils/transformations";

import {TSInsertRecordReq} from "../gen-nodejs/client_types";


export class IoTDBHandler extends HandlerBase {
  private session: Session;
  private subscriptionSimulator: SubscriptionSimulator;
  private dataPointsSchema: SupportedDataPoints = {};

  constructor() {
    super();
    if (!databaseConfig) {
      throw new Error("Invalid database configuration.");
    }
    this.session = new Session();
    this.subscriptionSimulator = getSubscriptionSimulator(
      this.sendMessageToClient,
      this.createDataContentMessage,
      this.createStatusMessage,
      this.sendAlreadySubscribedErrorMsg);
  }

  async authenticateAndConnect(): Promise<void> {
    await this.session.authenticateAndConnect();
    const supportedDataPoint: SupportedDataPoints =
      this.getSupportedDataPoints() as SupportedDataPoints;
    this.dataPointsSchema = createDataPointsSchema(supportedDataPoint);
    await this.createDatabaseIfNeeded()
   }
  
  async createDatabaseIfNeeded() {
    const sql = `CREATE DATABASE ${databaseParams["VSS"].databaseName};`;
    try {
      await this.session.executeQueryStatement(sql);
    } catch (e) {
      logErrorStr(`Error creating database in IotDB: ${e}`);
    }
  }

  protected subscribe(message: SubscribeMessageType, ws: WebSocketWithId): void {
    const newDataPoints = this.getKnownDatapointsByPrefix(message.path);

    if (newDataPoints.length === 0) {
      this.sendRequestedDataPointsNotFoundErrorMsg(ws, message.path)
      return;
    }

    void this.subscriptionSimulator.subscribe(message, ws, newDataPoints);
  }

  protected unsubscribe(message: UnsubscribeMessageType, ws: WebSocketWithId): void {
    const dataPointsToUnsub = this.getKnownDatapointsByPrefix(message.path);
    if (dataPointsToUnsub.length === 0) {
      this.sendRequestedDataPointsNotFoundErrorMsg(ws, message.path)
      return;
    }

    this.subscriptionSimulator.unsubscribe(message, ws, dataPointsToUnsub);
  }

  async unsubscribe_client(ws: WebSocketWithId): Promise<void> {
    this.subscriptionSimulator.unsubscribeClient(ws);
    await this.session.closeSession();
  }

  protected async set(message: SetMessageType, ws: WebSocketWithId): Promise<void> {
    if (this.areNodesValid(message, ws)) {
      let statusMessage: StatusMessage;
      try {
        const data = {
          ...this.extractNodesFromMessageWithVinAsNode(message),
          ...this.extractNodesFromMetadata(message)
        }
        let measurements: string[] = [];
        let dataTypes: string[] = [];
        let values: any[] = [];

        for (const [key, value] of Object.entries(data)) {
          measurements.push(key);
          dataTypes.push(this.getDataType(key));
          values.push(value);
        }

        const deviceId = databaseParams["VSS"].databaseName;
        const status = await this.insertRecord(
          deviceId,
          measurements,
          dataTypes,
          values
        );

        logWithColor(`Record inserted to device ${deviceId}, 
          status code: `.concat(JSON.stringify(status)),
          COLORS.GREY
        );

        statusMessage = this.createStatusMessage(STATUS_SUCCESS.OK, "Successfully wrote data to database.");
      } catch (error: unknown) {
        const errMsg = error instanceof Error ? error.message : "Unknown error";
        statusMessage = this.createStatusMessage(STATUS_ERRORS.SERVICE_UNAVAILABLE, `Failed writing data. ${errMsg}`);
      }
      this.sendMessageToClient(ws, statusMessage);
    }
  }

  private extractNodesFromMetadata(message: SetMessageType): Record<string, any> {
    if (!message.metadata) return {}; // Return an empty object if metadata is missing

    return Object.fromEntries(
      Object.entries(message.metadata).map(([key, value]) =>
        [
          message.path + this.formatKey(key) + METADATA_SUFFIX,
          JSON.stringify(value)
        ]
      )
    );
  }
  
  private formatKey = (key: string) => key ? "_" + replaceDotsWithUnderscore(key) : "";

  private getDataType(dataPointName: string) {
    // return string type for metadata if correlating data point is known, throw error otherwise
    if (dataPointName.endsWith(METADATA_SUFFIX)) {
      let dataPoint = removeSuffixFromString(dataPointName, METADATA_SUFFIX);
      if (this.dataPointsSchema.hasOwnProperty(dataPoint)) {
        return SupportedMessageDataTypes.string
      } else {
        throw new Error(`Invalid metadata provided, datapoint ${replaceUnderscoresWithDots(dataPoint)} not defined.`);
      }
    }

    // return defined datapoint type 
    return this.dataPointsSchema[dataPointName];
  }

  /**
   * Validates the nodes in a message against the schema of a media element.
   *
   * @param message - The message object containing details for the request.
   * @param ws - The WebSocket object for communication.
   * @returns - Returns true if all nodes are valid against the schema, otherwise false.
   */
  private areNodesValid(message: NewMessage, ws: WebSocketWithId): boolean {
    const errorMessage = this.validateNodesAgainstSchema(
      message,
      this.dataPointsSchema
    );

    if (errorMessage) {
      logErrorStr(`Error validating message nodes against schema: ${errorMessage}`);
      const statusMessage = this.createStatusMessage(STATUS_ERRORS.NOT_FOUND, errorMessage);
      this.sendMessageToClient(ws, statusMessage);
      return false;
    }
    return true;
  }

  /**
   * Inserts a record into the time series database.
   * @param deviceId - The ID of the device.
   * @param measurements - Array of measurement names.
   * @param dataTypes - Array of data types for each value.
   * @param values - Array of values to be inserted.
   * @param isAligned - Flag indicating if the data is aligned.
   * @returns - A promise that resolves with the result of the insertion.
   * @throws - Throws an error if lengths of data types, values, and measurements do not match.
   */
  private async insertRecord(
    deviceId: string,
    measurements: string[],
    dataTypes: string[],
    values: any[],
    isAligned = false
  ): Promise<any> {
    if (!this.session.getSessionId()) {
      throw new Error("Session is not open. Please authenticate first.");
    }
    if (
      values.length !== dataTypes.length ||
      values.length !== measurements.length
    ) {
      throw "Length of data types does not equal to length of values!";
    }

    // Transform measurements (paths) from dots to underscores
    const transformedMeasurements = measurements.map((measurement) =>
      replaceDotsWithUnderscore(measurement)
    );

    // Validate the dataTypes before using them
    const validatedDataTypes: (keyof typeof SupportedMessageDataTypes)[] = [];

    dataTypes.forEach((dataType) => {
      if (isSupportedDataPoint(dataType)) {
        validatedDataTypes.push(dataType); // Add valid data types
      } else {
        throw new Error(`Unsupported data type: ${dataType}`);
      }
    });

    const valuesInBytes = IoTDBDataInterpreter.serializeValues(
      validatedDataTypes,
      values
    );

    const request = new TSInsertRecordReq({
      sessionId: this.session.getSessionId()!,
      prefixPath: deviceId,
      measurements: transformedMeasurements,
      values: valuesInBytes,
      timestamp: Date.now(),
      isAligned: isAligned,
    });

    return await this.session.getClient().insertRecord(request);
  }

  async getDataPointsFromDB(
    dataPoints: string[],
    vin: string
  ): Promise<QueryResult> {

    const metadataPoints = dataPoints.map((dataPoint) => dataPoint + METADATA_SUFFIX)
    const {databaseName, dataPointId} = databaseParams["VSS"];
    const latestDataPoints: Array<{ name: string; value: any }> = [];
    const latestMetadata: Array<{ name: string; value: any }> = [];
    try {
      for (let i = 0; i < dataPoints.length; i++) {
        const dataPoint = dataPoints[i];
        const metadataPoint = metadataPoints[i];
        const fieldSQL = `SELECT ${dataPoint + "," + metadataPoint}
                          FROM ${databaseName}
                          WHERE ${dataPointId} = '${vin}'
                            AND ${dataPoint} IS NOT NULL
                          ORDER BY time DESC
                              LIMIT 1`;
        const sessionDataSet = await this.session.executeQueryStatement(fieldSQL);
        if (sessionDataSet instanceof SessionDataSet) {
          const [data, meta] = transformSessionDataSet(sessionDataSet, databaseName);
          data.forEach(({name, value}) => {
            latestDataPoints.push({name, value});
          });

          meta.forEach(({name, value}) => {
            latestMetadata.push({name, value});
          });
        }
      }

      return {success: true, dataPoints: latestDataPoints, metadata: latestMetadata};

    } catch (error: unknown) {
      const errMsg = error instanceof Error ? error.message : "Unknown database error";
      return {success: false, error: errMsg};
    }
  }

  getKnownDatapointsByPrefix(datapointPrefix: string) {
    const allKnownDataPoints: string[] = Object.keys(this.dataPointsSchema);
    return allKnownDataPoints
      .filter(field => field !== databaseParams["VSS"].dataPointId) // remove VIN from the list, so only real data points are left
      .filter(value => value === datapointPrefix
        || (value.startsWith(datapointPrefix) && value[datapointPrefix.length] === "_")
      );
  }
}
