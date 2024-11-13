import thrift from "thrift";
import { Session } from "./Session";
import { SubscriptionSimulator } from "./SubscriptionSimulator";
import { SupportedMessageDataTypes } from "./../utils/iotdb-constants";
const Client = require("../gen-nodejs/IClientRPCService");
const {
  TSInsertRecordReq,
}: any = require("../gen-nodejs/client_types");
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
  COLORS,
} from "../../../../utils/logger";
import { createErrorMessage } from "../../../../utils/error-message-helper";
import { WebSocket, Message, STATUS_ERRORS, WebSocketWithId } from "../../../utils/data-types";
import { transformSessionDataSet } from "../utils/database-helper";
import { transformDataPointsWithUnderscores } from "../../../utils/transformations";

export class IoTDBHandler extends HandlerBase {
  private client: any = null;
  private sendMessageToClients:
    | ((ws: WebSocket, message: Message) => void)
    | null = null;
  private session: Session;
  private subscriptionSimulator: SubscriptionSimulator;
  private dataPointsSchema: SupportedDataPoints = {};

  constructor() {
    super();
    if (!databaseConfig) {
      throw new Error("Invalid database configuration.");
    }
    this.session = new Session();
    this.subscriptionSimulator = new SubscriptionSimulator(this.session, this.sendMessageToClient, this.createUpdateMessage);
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
      this.session.setClient(this.client);

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

  protected subscribe(message: Message, ws: WebSocketWithId): void {
    this.subscriptionSimulator.subscribe(message, ws);
  }

  protected unsubscribe(message: Message, ws: WebSocketWithId): void {
    this.subscriptionSimulator.unsubscribe(message, ws);
  }

  unsubscribe_client(ws: WebSocketWithId): void {
     this.subscriptionSimulator.unsubscribeClient(ws);
  }
  
  protected async read(message: Message, ws: WebSocket): Promise<void> {
    if (this.areNodesValid(message, ws)) {
      try {
        await this.session.openSession();
        const responseNodes = await this.queryLastFields(message, ws);
        if (responseNodes.length > 0) {
          const responseMessage = this.createUpdateMessage(
            message.id, message.tree, message.uuid,
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
        await this.session.closeSession();
      }
    }
  }

  protected async write(message: Message, ws: WebSocket): Promise<void> {
    if (this.areNodesValid(message, ws)) {
      try {
        await this.session.openSession();
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
            message.id, message.tree, message.uuid,
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
        await this.session.closeSession();
      }
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
      const sessionDataSet = await this.session.executeQueryStatement(sql);

      // Check if sessionDataSet is not an instance of SessionDataSet, and handle the error
      if (!(sessionDataSet instanceof SessionDataSet)) {
        throw new Error(
          "Failed to retrieve session data. Invalid session dataset."
        );
      }
      return transformSessionDataSet(sessionDataSet, databaseName);
      
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
        transformDataPointsWithUnderscores(message.node.name)
      );
    } else if (message.nodes) {
      dataPoints = message.nodes.map((node) =>
        transformDataPointsWithUnderscores(node.name)
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
      data[transformDataPointsWithUnderscores(message.node.name)] =
        message.node.value;
    } else if (message.nodes) {
      message.nodes.forEach((node) => {
        data[transformDataPointsWithUnderscores(node.name)] = node.value;
      });
    }
    return data;
  }
}