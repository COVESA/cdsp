import { SupportedMessageDataTypes } from "./../utils/iotdb-constants";
const Client = require("../gen-nodejs/IClientRPCService");
const {
  TSExecuteStatementReq,
  TSOpenSessionReq,
  TSProtocolVersion,
  TSCloseSessionReq,
  TSInsertRecordReq,
}: any = require("../gen-nodejs/client_types");
import thrift from "thrift";
import { HandlerBase } from "../../HandlerBase";
import { SessionDataSet } from "../utils/SessionDataSet";
import { IoTDBDataInterpreter } from "../utils/IoTDBDataInterpreter";
import {
  createDataPointsSchema,
  isSupportedDataPoint,
  SupportedDataPoints,
} from "../config/iotdb-config";
import { databaseParams, databaseConfig } from "../config/database-params";
import {
  logMessage,
  logError,
  logErrorStr,
  logWithColor,
  MessageType,
  COLORS,
} from "../../../../utils/logger";
import { createErrorMessage } from "../../../../utils/error-message-helper";
import { WebSocket, Message, STATUS_ERRORS } from "../../../utils/data_types";

export class IoTDBHandler extends HandlerBase {
  private client: any = null;
  private sendMessageToClients:
    | ((ws: WebSocket, message: Message) => void)
    | null = null;
  private sessionId: number | null = null;
  private readonly protocolVersion: any =
    TSProtocolVersion.IOTDB_SERVICE_PROTOCOL_V3;
  private statementId: number | null = null;
  private fetchSize: number;
  private dataPointsSchema: SupportedDataPoints = {};

  constructor() {
    super();
    if (!databaseConfig) {
      throw new Error("Invalid database configuration.");
    }
    this.fetchSize = databaseConfig?.fetchSize ?? 1000;
  }

  async authenticateAndConnect(
    sendMessageToClients: (ws: WebSocket, message: Message) => void
  ): Promise<void> {
    try {
      this.sendMessageToClients = sendMessageToClients;

      const connection = thrift.createConnection(
        databaseConfig!.iotdbHost,
        databaseConfig!.iotdbPort!,
        {
          transport: thrift.TFramedTransport,
          protocol: thrift.TBinaryProtocol,
        }
      );

      logMessage(
        `Connect to IoTDB, host: ${databaseConfig!.iotdbHost} port: ${databaseConfig!.iotdbPort}`
      );

      this.client = thrift.createClient(Client, connection);

      connection.on("error", (error: Error) => {
        logError("thrift connection error", error);
      });

      const supportedDataPoint: SupportedDataPoints =
        this.getSupportedDataPoints() as SupportedDataPoints;
      this.dataPointsSchema = createDataPointsSchema(supportedDataPoint);

      logMessage("Successfully connected to IoTDB using thrift");
    } catch (error: unknown) {
      logError("Failed to authenticate with IoTDB", error);
    }
  }

  protected async read(message: Message, ws: WebSocket): Promise<void> {
    if (this.areNodesValid(message, ws)) {
      try {
        await this.openSessionIfNeeded();
        const responseNodes = await this.queryLastFields(message, ws);
        if (responseNodes.length > 0) {
          const responseMessage = this.createUpdateMessage(
            message,
            responseNodes
          );
          this.sendMessageToClient(ws, responseMessage);
        } else {
          this.sendMessageToClient(
            ws,
            createErrorMessage(
              "read",
              STATUS_ERRORS.NOT_FOUND,
              `No data found with the Id: ${message.id}`
            )
          );
        }
      } catch (error: unknown) {
        const errMsg = error instanceof Error ? error.message : "Unknown error";
        this.sendMessageToClient(
          ws,
          createErrorMessage("read", STATUS_ERRORS.NOT_FOUND, errMsg)
        );
      } finally {
        await this.closeSessionIfNeeded();
      }
    }
  }

  protected async write(message: Message, ws: WebSocket): Promise<void> {
    if (this.areNodesValid(message, ws)) {
      try {
        await this.openSessionIfNeeded();
        const data = this.createObjectToInsert(message);
        let measurements: string[] = [];
        let dataTypes: string[] = [];
        let values: any[] = [];

        for (const [key, value] of Object.entries(data)) {
          measurements.push(key);
          dataTypes.push(this.dataPointsSchema[key]);
          values.push(value);
        }

        const tree = message.tree as keyof typeof databaseParams;
        if (!tree || !databaseParams[tree]) {
          throw new Error(`Invalid tree specified: ${message.tree}`);
        }

        const deviceId = databaseParams[tree].databaseName;
        const status = await this.insertRecord(
          deviceId,
          measurements,
          dataTypes,
          values
        );

        logWithColor(
          `Record inserted to device ${deviceId}, status code: `.concat(
            JSON.stringify(status)
          ),
          COLORS.GREY
        );

        const responseNodes = await this.queryLastFields(message, ws);

        if (responseNodes.length) {
          const responseMessage = this.createUpdateMessage(
            message,
            responseNodes
          );
          this.sendMessageToClient(ws, responseMessage);
        } else {
          this.sendMessageToClient(
            ws,
            createErrorMessage(
              "write",
              STATUS_ERRORS.NOT_FOUND,
              `No data found with the Id: ${message.id}`
            )
          );
        }
      } catch (error: unknown) {
        const errMsg = error instanceof Error ? error.message : "Unknown error";
        this.sendMessageToClient(
          ws,
          createErrorMessage(
            "write",
            STATUS_ERRORS.SERVICE_UNAVAILABLE,
            `Failed writing data. ${errMsg}`
          )
        );
      } finally {
        await this.closeSessionIfNeeded();
      }
    }
  }

  /**
   * Opens a session with the IoTDB server using the provided credentials and configuration.
   */
  private async openSession(): Promise<void> {
    if (this.sessionId) {
      logMessage("The session is already opened.");
      return;
    }

    const openSessionReq = new TSOpenSessionReq({
      username: databaseConfig!.iotdbUser,
      password: databaseConfig!.iotdbPassword,
      client_protocol: this.protocolVersion,
      zoneId: databaseConfig!.timeZoneId,
      configuration: { version: "V_0_13" },
    });

    try {
      if (!this.client) {
        throw new Error("Client is not initialized");
      }
      const resp = await this.client.openSession(openSessionReq);

      if (this.protocolVersion !== resp.serverProtocolVersion) {
        logMessage(
          "Protocol differ, Client version is " +
            this.protocolVersion +
            ", but Server version is " +
            resp.serverProtocolVersion
        );
        // version is less than 0.10
        if (resp.serverProtocolVersion === 0) {
          throw new Error("Protocol not supported.");
        }
      }

      this.sessionId = resp.sessionId;

      if (!this.sessionId) {
        throw new Error(
          "Session ID is undefined, cannot request statement ID."
        );
      }

      this.statementId = await this.client.requestStatementId(this.sessionId);
      logMessage("Session started!");
    } catch (error: unknown) {
      logError("Failed starting session with IoTDB", error);
    }
  }

  /**
   * Closes the current session if it is not already closed.
   */
  private async closeSession(): Promise<void> {
    if (!this.sessionId) {
      logMessage("Session is already closed.");
      return;
    }

    const req = new TSCloseSessionReq({ sessionId: this.sessionId! });

    try {
      await this.client.closeSession(req);
    } catch (error: unknown) {
      logError(
        "Error occurs when closing session at server. Maybe server is down. Error message",
        error
      );
    } finally {
      this.sessionId = null;
      logMessage("Session closed!");
    }
  }

  private async openSessionIfNeeded(): Promise<void> {
    if (!this.sessionId) {
      await this.openSession();
    }
  }

  private async closeSessionIfNeeded(): Promise<void> {
    if (this.sessionId) {
      this.closeSession();
    }
  }

  /**
   * Validates the nodes in a message against the schema of a media element.
   *
   * @param message - The message object containing details for the request.
   * @param ws - The WebSocket object for communication.
   * @returns - Returns true if all nodes are valid against the schema, otherwise false.
   */
  private areNodesValid(message: Message, ws: WebSocket): boolean {
    const { type } = message;

    const errorData = this.validateNodesAgainstSchema(
      message,
      this.dataPointsSchema
    );

    if (errorData) {
      logErrorStr(
        `Error validating message nodes against schema: ${JSON.stringify(errorData)}`
      );
      this.sendMessageToClient(
        ws,
        createErrorMessage(
          `${type}`,
          STATUS_ERRORS.NOT_FOUND,
          JSON.stringify(errorData)
        )
      );

      return false;
    }
    return true;
  }

  /**
   * Executes a SQL query statement asynchronously.
   *
   * @param sql - The SQL query statement to be executed.
   * @returns - Returns a SessionDataSet object if the query is successful,
   *                                             otherwise returns an empty object.
   * @throws - Throws an error if the session is not open.
   */
  private async executeQueryStatement(
    sql: string
  ): Promise<SessionDataSet | {}> {
    try {
      if (!this.sessionId) {
        throw new Error("Session is not open. Please authenticate first.");
      }
      if (!this.statementId) {
        throw new Error("Missing statement ID");
      }

      const request = new TSExecuteStatementReq({
        sessionId: this.sessionId,
        statement: sql,
        statementId: this.statementId,
        fetchSize: this.fetchSize,
        timeout: 0,
      });

      const resp = await this.client.executeQueryStatement(request);
      if (!resp || !resp.queryDataSet || !resp.queryDataSet.valueList) {
        return {};
      }
      return new SessionDataSet(
        resp.columns,
        resp.dataTypeList,
        resp.columnNameIndexMap,
        resp.queryId,
        this.client,
        this.statementId,
        this.sessionId,
        resp.queryDataSet,
        resp.ignoreTimeStamp
      );
    } catch (error: unknown) {
      logError("Failed executing query statement", error);
      throw error;
    }
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
    if (!this.sessionId) {
      throw new Error("Session is not open. Please authenticate first.");
    }
    if (
      values.length !== dataTypes.length ||
      values.length !== measurements.length
    ) {
      throw "Length of data types does not equal to length of values!";
    }

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
      sessionId: this.sessionId!,
      prefixPath: deviceId,
      measurements: measurements,
      values: valuesInBytes,
      timestamp: Date.now(),
      isAligned: isAligned,
    });

    return await this.client.insertRecord(request);
  }

  /**
   * Queries the last fields from the database based on the provided message and sends the response to the client.
   *
   * @param message - The message object containing the query details.
   * @param ws - The WebSocket connection to send the response to.
   */
  private async queryLastFields(
    message: Message,
    ws: WebSocket
  ): Promise<Array<{ name: string; value: any }>> {
    const { id: objectId, tree } = message;

    if (!tree || !(tree in databaseParams)) {
      const errorMsg = `Invalid or undefined tree provided: ${tree}`;
      logErrorStr(errorMsg);
      this.sendMessageToClient(
        ws,
        createErrorMessage("read", STATUS_ERRORS.NOT_FOUND, errorMsg)
      );
      return [];
    }
    const { databaseName, dataPointId } =
      databaseParams[tree as keyof typeof databaseParams];

    const fieldsToSearch = this.extractDataPointsFromNodes(message).join(", ");
    const sql = `SELECT ${fieldsToSearch} FROM ${databaseName} WHERE ${dataPointId} = '${objectId}' ORDER BY Time ASC`;

    try {
      const sessionDataSet = await this.executeQueryStatement(sql);

      // Check if sessionDataSet is not an instance of SessionDataSet, and handle the error
      if (!(sessionDataSet instanceof SessionDataSet)) {
        throw new Error(
          "Failed to retrieve session data. Invalid session dataset."
        );
      }

      const mediaElements: any[] = [];
      while (sessionDataSet.hasNext()) {
        mediaElements.push(sessionDataSet.next());
      }

      const latestValues: Record<string, any> = {};
      mediaElements.forEach((mediaElement) => {
        const transformedMediaElement = Object.fromEntries(
          Object.entries(mediaElement).map(([key, value]) => {
            const newKey = this.transformDataPointsWithDots(key);
            return [newKey, value];
          })
        );

        const transformedObject =
          IoTDBDataInterpreter.extractNodesFromTimeseries(
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
    } catch (error: unknown) {
      const errMsg = error instanceof Error ? error.message : "Unknown error";
      this.sendMessageToClient(
        ws,
        createErrorMessage("read", STATUS_ERRORS.SERVICE_UNAVAILABLE, errMsg)
      );
      return [];
    }
  }

  /**
   * Extracts data point names from the given message.
   *
   * This function checks if the message has a single node or multiple nodes and
   * extracts the names accordingly.
   *
   * @param message - The message containing node(s).
   * @returns An array of data point names.
   */
  private extractDataPointsFromNodes(message: Message): string[] {
    let dataPoints: string[] = [];

    if (message.node) {
      dataPoints.push(
        this.transformDataPointsWithUnderscores(message.node.name)
      );
    } else if (message.nodes) {
      dataPoints = message.nodes.map((node) =>
        this.transformDataPointsWithUnderscores(node.name)
      );
    }
    return dataPoints;
  }

  /**
   * Creates an object to be inserted into the database based on the provided message.
   * The object is constructed using the message's ID and its associated nodes.
   *
   * @param message - The message object containing the ID, tree, and nodes.
   * @returns An object representing the data to be inserted.
   * @throws Will throw an error if the tree is invalid or undefined.
   */
  private createObjectToInsert(message: Message): Record<string, any> {
    const { id, tree } = message;

    if (!tree || !(tree in databaseParams)) {
      throw new Error(`Invalid or undefined tree provided: ${tree}`);
    }
    const { dataPointId } = databaseParams[tree as keyof typeof databaseParams];
    const data: Record<string, any> = { [dataPointId]: id };

    if (message.node) {
      data[this.transformDataPointsWithUnderscores(message.node.name)] =
        message.node.value;
    } else if (message.nodes) {
      message.nodes.forEach((node) => {
        data[this.transformDataPointsWithUnderscores(node.name)] = node.value;
      });
    }
    return data;
  }
}