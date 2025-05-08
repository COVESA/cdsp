import fs from "fs";
import yaml from "js-yaml";
import {
  DataContentMessage,
  ensureMessageType,
  GetMessageType,
  NewMessage,
  NewMessageType,
  SetMessageType,
  StatusMessage,
  SubscribeMessageType, UnsubscribeMessageType
} from "../../router/utils/NewMessage"
import {DataPointSchema, WebSocketWithId} from "../../utils/database-params"
import {ErrorMessage, STATUS_ERRORS} from "../../router/utils/ErrorMessage";
import {getDataPointsPath} from "../config/config";
import {logMessage, LogMessageType} from "../../utils/logger";
import {replaceDotsWithUnderscore, replaceUnderscoresWithDots, toResponseFormat} from "../utils/transformations";
import {databaseParams} from "./iotdb/config/database-params";

export abstract class HandlerBase {

  /**
   *  Returns a list of all known data point names that begin with the specified prefix.
   */
  abstract getKnownDatapointsByPrefix(prefix: string): string[]

  /**
   * Returns the dataPoints from DB based on the provided list of dataPoints and VIN.
   * @param dataPoints to extract from DB
   * @param vin VIN
   */
  abstract getDataPointsFromDB(dataPoints: string[], vin: string): Promise<QueryResult>;

  // Default implementations of required functions
  async authenticateAndConnect(): Promise<void> {
    logMessage(
      "authenticateAndConnect() is not implemented",
      LogMessageType.WARNING
    );
  }

  async get(message: GetMessageType, ws: WebSocketWithId): Promise<void> {
    const requestedDataPoints = this.getKnownDatapointsByPrefix(message.path);

    if (requestedDataPoints.length === 0) { // no valid datapoints found for requested path 
      this.sendRequestedDataPointsNotFoundErrorMsg(ws, message.path);
      return;
    }

    // Access the DB for the datapoints and their values
    const queryResult = await this.getDataPointsFromDB(requestedDataPoints, message.instance);

    this.sendGetResponseToClient(queryResult, message.instance, requestedDataPoints, ws);
  }

  protected sendRequestedDataPointsNotFoundErrorMsg(ws: WebSocketWithId, path: string) {
    this.sendMessageToClient(ws, this.createStatusMessage(
      STATUS_ERRORS.BAD_REQUEST,
      `There are no known datapoints with prefix ${replaceUnderscoresWithDots(path)}`
    ));
  }

  protected sendAlreadySubscribedErrorMsg(ws: WebSocketWithId, instance: string, newDataPoints: string[]) {
    const statusMessage = this.createStatusMessage(STATUS_ERRORS.BAD_REQUEST,
      `Subscription already done to instance '${instance}' ` +
      `and datapoints [${toResponseFormat(newDataPoints)}].`);
    this.sendMessageToClient(ws, statusMessage);
  }

  protected set(message: SetMessageType, ws: WebSocketWithId): void {
    logMessage("set() is not implemented", LogMessageType.WARNING);
  }

  protected subscribe(message: SubscribeMessageType, ws: WebSocketWithId): void {
    logMessage("subscribe() is not implemented", LogMessageType.WARNING);
  }

  protected unsubscribe(message: UnsubscribeMessageType, ws: WebSocketWithId): void {
    logMessage("unsubscribe() is not implemented", LogMessageType.WARNING);
  }

  unsubscribe_client(ws: WebSocketWithId): void {
    logMessage("unsubscribe_client() is not implemented", LogMessageType.WARNING);
  }

  handleMessage(message: NewMessage, ws: WebSocketWithId): void {
    try {
      switch (message.type) {
        case "get":
          const aGetMessage = ensureMessageType<GetMessageType>(message, NewMessageType.Get);
          void this.get(aGetMessage, ws);
          break;
        case "set":
          const aSetMessage = ensureMessageType<SetMessageType>(message, NewMessageType.Set);
          this.set(aSetMessage, ws);
          break;
        case "subscribe":
          const aSubscribeMessage = ensureMessageType<SubscribeMessageType>(message, NewMessageType.Subscribe);
          this.subscribe(aSubscribeMessage, ws);
          break;
        case "unsubscribe":
          const anUnsubscribeMessage = ensureMessageType<UnsubscribeMessageType>(message, NewMessageType.Unsubscribe);
          this.unsubscribe(anUnsubscribeMessage, ws);
          break;
        default:
          ws.send(JSON.stringify({error: "Unknown message type"}));
      }
    } catch (error: unknown) {
      const errMsg = error instanceof Error ? error.message : "Unknown error";
      ws.send(errMsg);
    }
  }

  /**
   * Takes the queryResults and sends a response to the client. 3 different responses are possible:
   * 1. Valida response with all available datapoints in the DB (number of datapoints can be less than requested)
   * 2. Client Error 404 if no requested datapoint could be found.
   * 3. Internal Server Error 500 if the DB access was not successful.
   *
   */
  protected sendGetResponseToClient(
    queryResult: QueryResult,
    vin: string,
    requestedDataPoints: string[],
    ws: WebSocketWithId): void {
    // There was an issue accessing the DB, notify the client and return
    if (!queryResult.success) {
      this.sendMessageToClient(ws, this.createStatusMessage(STATUS_ERRORS.INTERNAL_SERVER_ERROR, queryResult.error));
      return;
    }

    let responseMessage: DataContentMessage | StatusMessage;
    if (queryResult.dataPoints.length > 0) {
      responseMessage = this.createDataContentMessage(vin, queryResult.dataPoints, queryResult.metadata);
    } else {
      responseMessage = this.createStatusMessage(
        STATUS_ERRORS.NOT_FOUND,
        `No values found for dataPoints [${replaceUnderscoresWithDots(requestedDataPoints.join(", "))}] for the instance: ${vin}`
      );
    }

    this.sendMessageToClient(ws, responseMessage);
  }

  /**
   * Sends a message to the client.
   */
  protected sendMessageToClient(
    ws: WebSocketWithId,
    message: StatusMessage | DataContentMessage | ErrorMessage
  ): void {
    logMessage(JSON.stringify(message), LogMessageType.SENT, ws.id);
    ws.send(JSON.stringify(message));
  }

  /**
   * Generic function to create a status message.
   * @param code - status code (http-based).
   * @param statusMessage - A descriptive message.
   * @param requestId - The ID of the corresponding request.
   * @returns - A status message.
   */
  protected createStatusMessage(
    code: number,
    statusMessage: string,
    requestId?: string
  ): StatusMessage {
    return {
      type: "status", // Constant string
      code, // Status code
      message: statusMessage, // Status message
      ...(requestId && {requestId}), // Include requestId only if provided
      timestamp: getCurrentTimestamp(), // Generated timestamp
    };
  }

  /**
   * Generic function to create a data content message.
   * @param instance - The ID of the element in the tree.
   * @param dataPoints - the data points with values.
   * @param metadata - available metadata for the data points
   * @param requestId - The ID of the corresponding request.
   * @param shouldBeNested - Controls format of the data response structure: Nested if true, flat if false.
   * @returns - A data content message.
   */
  protected createDataContentMessage(
    instance: string,
    dataPoints: Array<{ name: string; value: any }>,
    metadata?: Array<{ name: string; value: any }>,
    requestId?: string,
    shouldBeNested: boolean = false
  ): DataContentMessage {
    // Ensure nodes are valid
    if (dataPoints.length === 0) {
      throw new Error("Nodes array cannot be empty.");
    }

    return {
      type: "data",
      instance: instance,
      schema: getSchemaOrThrow(dataPoints),
      data: shouldBeNested ? buildDataStructureAsTreeWithoutRootElement(dataPoints) : buildDataStructureFlatWithoutRootElement(dataPoints),
      requestId: requestId,
      ...(metadata && Object.keys(metadata).length && {metadata: buildDataStructureFlatWithoutRootElement(metadata)}),
    };
  }


  protected extractNodesFromData(path: string, data: Record<string, any>): Record<string, any> {
    const result: Record<string, any> = {};

    function traverse(currentPath: string, currentData: Record<string, any>): void {
      for (const [key, value] of Object.entries(currentData)) {
        const fullPath = `${currentPath}.${key}`;
        if (typeof value === "object" && value !== null) {
          traverse(fullPath, value); // Recursive call for nested objects
        } else {
          result[fullPath] = value; // Add leaf node to the result
        }
      }
    }

    traverse(path, data);
    return result;
  }

  /**
   * Reads and parses a data points file in either JSON, YML, or YAML format.
   */
  private readDataPointsFile(filePath: string): object {
    const fileContent = fs.readFileSync(filePath, "utf8");
    const filePathLower = filePath.toLowerCase();
    if (filePathLower.endsWith(".json")) {
      return JSON.parse(fileContent);
    } else if (
      filePathLower.endsWith(".yaml") ||
      filePathLower.endsWith(".yml")
    ) {
      const result = yaml.load(fileContent);
      if (typeof result === "object" && result !== null) {
        return result;
      } else {
        throw new Error("YAML content is not a valid object");
      }
    } else {
      throw new Error("Unsupported data points file format");
    }
  }

  /**
   * Extracts data types from a data point object.
   */
  private extractDataTypes(
    dataPointsObj: any,
    parentKey = "",
    result: { [key: string]: any } = {}
  ): { [key: string]: any } {
    for (const key in dataPointsObj) {
      if (dataPointsObj.hasOwnProperty(key)) {
        const value = dataPointsObj[key];
        const newKey = parentKey ? `${parentKey}.${key}` : key;
        const isObject = value && typeof value === "object";
        if (isObject && value.datatype) {
          result[newKey] = value.datatype;
        } else if (isObject) {
          this.extractDataTypes(value.children || value, newKey, result);
        }
      }
    }
    return result;
  }

  /**
   * Retrieves and processes supported data points.
   * This method reads the data points configuration file, extracts the data types,
   * and transforms the data point names to use underscores. It returns an object
   * with the transformed data point names as keys and their corresponding data types.
   * @returns An object containing the supported data points with transformed names and data types.
   */
  protected getSupportedDataPoints(): object {
    const dataPointPath = getDataPointsPath();
    const dataPointObj = this.readDataPointsFile(dataPointPath);
    const supportedDataPoints = this.extractDataTypes(dataPointObj);
    const result: { [key: string]: any } = {};
    Object.entries(supportedDataPoints).forEach(([node, value]) => {
      const underscored_node = replaceDotsWithUnderscore(node);
      if (value !== null) {
        result[underscored_node] = value;
      }
    });
    return result;
  }

  /**
   * Validates nodes against a given schema.
   *
   * @param message - The message containing a path to be validated.
   * @param dataPointsSchema - The schema against which nodes are validated.
   * @returns An error message with details if validation fails, otherwise null.
   */
  protected validateNodesAgainstSchema(
    message: NewMessage,
    dataPointsSchema: DataPointSchema
  ): string | null {
    // build the nodes from path and data
    if (message.type == NewMessageType.PermissionsEdit) {
      return null;
    }
    const data = message.type === NewMessageType.Set ? message.data : undefined;
    const requestNodes = extractLeafNodesFromRequest(message.path, data);

    const unknownFields = Array.from(requestNodes).filter(node => {
      return !dataPointsSchema.hasOwnProperty(node);
    });

    if (unknownFields.length > 0) {
      return "Could not find node: " + replaceUnderscoresWithDots(unknownFields.join(", "));
    }
    return null;
  }

  protected extractNodesFromMessage(message: SetMessageType): Record<string, any> {
    const result: Record<string, any> = {};

    if (typeof message.data === "object" && message.data !== null) {
      // Case: data is a nested object or key-value structure
      const nodes = this.extractNodesFromData(message.path, message.data);
      for (const [key, value] of Object.entries(nodes)) {
        result[replaceDotsWithUnderscore(key)] = value;
      }
    } else {
      // Case: data is a single value
      result[replaceDotsWithUnderscore(message.path)] = message.data;
    }

    return result;
  }

  protected extractNodesFromMessageWithVinAsNode(message: SetMessageType): Record<string, any> {
    const result = this.extractNodesFromMessage(message);
    // Add VIN as Node
    const {dataPointId} = databaseParams["VSS"];
    result[dataPointId] = message.instance;
    return result;
  }
}

function extractLeafNodesFromRequest(path: string, data?: any): Set<string> {
  const leafNodes = new Set<string>();

  if (data && typeof data === "object") {
    for (const [key, value] of Object.entries(data)) {
      const fullPath = `${path}.${key}`;
      if (typeof value === "object" && value !== null) {
        // Recursive call for nested objects
        const childLeafNodes = extractLeafNodesFromRequest(fullPath, value);
        for (const child of childLeafNodes) {
          leafNodes.add(child); // Add all child leaf nodes
        }
      } else {
        // If it's not an object, it's a leaf node
        leafNodes.add(fullPath);
      }
    }
  } else {
    // If data is not an object, the base path itself is a leaf node
    leafNodes.add(path);
  }

  return new Set<string>(
    Array.from(leafNodes, (value) => replaceDotsWithUnderscore(value))
  );
}

/**
 * Return the schema that is always the prefix (until the first dot) of all node names.
 * Throw in case of error
 * @param nodes list of nodes with names and values
 */
function getSchemaOrThrow(nodes: Array<{ name: string; value: any }>) {
  return nodes[0]?.name.split(".")[0] ?? (() => {
    throw new Error("Nodes array is empty");
  })();
}

// Helper function to build the flat data structure string without the root element
function buildDataStructureFlatWithoutRootElement(
  nodes: Array<{ name: string; value: any }>
): Record<string, any> {
  return nodes.reduce((accumulator, {name, value}) => {
    const nameWithoutRoot = name.includes(".") ? name.split(".").slice(1).join(".") : name;
    accumulator[nameWithoutRoot] = value;
    return accumulator;
  }, {} as Record<string, any>);
}

// Helper function to build the nested data structure string without the root element
function buildDataStructureAsTreeWithoutRootElement(
  nodes: Array<{ name: string; value: any }>,
): any {
  const data: any = {};
  const splitNames = nodes.map((node) => node.name.split("."));

  for (let i = 0; i < nodes.length; i++) {
    let currentLevel = data;
    const remainingPath = splitNames[i].slice(1); // Remaining part of the path after the common prefix

    for (let j = 0; j < remainingPath.length; j++) {
      const key = remainingPath[j];
      if (j === remainingPath.length - 1) {
        // Leaf node
        currentLevel[key] = nodes[i].value;
      } else {
        // Intermediate node
        if (!currentLevel[key]) {
          currentLevel[key] = {};
        }
        currentLevel = currentLevel[key];
      }
    }
  }

  return data;
}

function getCurrentTimestamp(): { seconds: number; nanos: number } {
  const now = new Date();
  const seconds = Math.floor(now.getTime() / 1000); // Convert milliseconds to seconds
  const nanos = (now.getTime() % 1000) * 1e6; // Remainder in milliseconds converted to nanoseconds
  return {seconds, nanos: nanos};
}

export type QueryResult =
  | { success: true; dataPoints: Array<{ name: string; value: any }>; metadata: Array<{ name: string; value: any }> }
  | { success: false; error: string };

