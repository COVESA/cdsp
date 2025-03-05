import {Session} from "./Session";
import {getSubscriptionSimulator, SubscriptionSimulator} from "./SubscriptionSimulator";
import {SupportedMessageDataTypes} from "../utils/iotdb-constants";
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
import {replaceDotsWithUnderscore} from "../../../utils/transformations";

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
        const data = this.extractNodesFromMessageWithVinAsNode(message);
        let measurements: string[] = [];
        let dataTypes: string[] = [];
        let values: any[] = [];

        for (const [key, value] of Object.entries(data)) {
          measurements.push(key);
          dataTypes.push(this.dataPointsSchema[key]);
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

    const {databaseName, dataPointId} = databaseParams["VSS"];
    const fieldsToSearch = dataPoints.join(", ");
    const sql = `SELECT ${fieldsToSearch}
                 FROM ${databaseName}
                 WHERE ${dataPointId} = '${vin}'
                 ORDER BY Time ASC`;

    try {
      const sessionDataSet = await this.session.executeQueryStatement(sql);

      // Check if sessionDataSet is not an instance of SessionDataSet, and handle the error
      if (!(sessionDataSet instanceof SessionDataSet)) {
        return {success: false, error: "Invalid session dataset received."};
      }
      return {success: true, nodes: transformSessionDataSet(sessionDataSet, databaseName)};

    } catch (error: unknown) {
      const errMsg = error instanceof Error ? error.message : "Unknown database error";
      return {success: false, error: errMsg};
    }
  }

  getKnownDatapointsByPrefix(datapointPrefix: string) {
    const allKnownDataPoints: string[] = Object.keys(this.dataPointsSchema);
    return allKnownDataPoints
      .filter(field => field !== databaseParams["VSS"].dataPointId) // remove VIN from the list, so only real data points are left
      .filter(value => value.startsWith(datapointPrefix));
  }
}
