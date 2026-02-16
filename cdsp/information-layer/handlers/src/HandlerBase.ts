import fs from "fs";
import yaml from "js-yaml";
import {
  DataContentMessage,
  ensureMessageType,
  ErrorMessage,
  STATUS_ERRORS,
  GetMessageType,
  NewMessage,
  NewMessageType,
  SetMessageType,
  StatusMessage,
  SubscribeMessageType,
  UnsubscribeMessageType,
} from "../../router/utils/NewMessage";
import { DataPointSchema, WebSocketWithId } from "../../utils/database-params";
import { getDataPointsPath } from "../config/config";
import { logMessage, LogMessageType } from "../../utils/logger";
import {
  replaceDotsWithUnderscore,
  replaceUnderscoresWithDots,
  toResponseFormat,
} from "../utils/transformations";
import { databaseParams } from "./iotdb/config/database-params";

export abstract class HandlerBase {
  /**
   * Function to use for sending messages to the client, so that formating and mapping is applied to all messages.
   * @private
   */
  private readonly sendMessage: (
    ws: WebSocketWithId,
    message: StatusMessage | DataContentMessage | ErrorMessage,
  ) => void;

  protected constructor(
    sendMessageFn: (
      ws: WebSocketWithId,
      message: StatusMessage | DataContentMessage | ErrorMessage,
    ) => void,
  ) {
    this.sendMessage = sendMessageFn;
  }

  /**
   *  Returns a list of all known data point names that begin with the specified prefix.
   */
  abstract getKnownDatapointsByPrefix(prefix: string): string[];

  /**
   * Returns the dataPoints from DB based on the provided list of dataPoints and VIN.
   * @param dataPoints to extract from DB
   * @param vin VIN
   */
  abstract getDataPointsFromDB(
    dataPoints: string[],
    vin: string,
  ): Promise<QueryResult>;

  // Default implementations of required functions
  async authenticateAndConnect(): Promise<void> {
    logMessage(
      "authenticateAndConnect() is not implemented",
      LogMessageType.WARNING,
    );
  }

  async get(message: GetMessageType, ws: WebSocketWithId): Promise<void> {
    const requestedDataPoints = this.getKnownDatapointsByPrefix(message.path);

    if (requestedDataPoints.length === 0) {
      // no valid datapoints found for requested path
      this.sendRequestedDataPointsNotFoundErrorMsg(
        ws,
        message.path,
        message.requestId,
      );
      return;
    }

    // Access the DB for the datapoints and their values
    const queryResult = await this.getDataPointsFromDB(
      requestedDataPoints,
      message.instance,
    );

    this.sendGetResponseToClient(
      queryResult,
      message.instance,
      requestedDataPoints,
      ws,
      message.requestId,
      message.path,
      message.root,
      message.format,
    );
  }

  protected sendRequestedDataPointsNotFoundErrorMsg(
    ws: WebSocketWithId,
    path: string,
    requestId: string,
  ) {
    this.sendMessageToClient(
      ws,
      this.createErrorMessage(
        STATUS_ERRORS.BAD_REQUEST,
        `Not found`,
        `There are no known datapoints with prefix ${replaceUnderscoresWithDots(path)}`,
        requestId,
      ),
    );
  }

  protected sendAlreadySubscribedErrorMsg(
    ws: WebSocketWithId,
    instance: string,
    newDataPoints: string[],
    requestId: string,
  ) {
    const statusMessage = this.createErrorMessage(
      STATUS_ERRORS.BAD_REQUEST,
      `Subscription failed`,
      `Already subscribed to instance '${instance}' ` +
        `and datapoints [${toResponseFormat(newDataPoints)}]`,
      requestId,
    );
    this.sendMessageToClient(ws, statusMessage);
  }

  protected set(message: SetMessageType, ws: WebSocketWithId): void {
    logMessage("set() is not implemented", LogMessageType.WARNING);
  }

  protected subscribe(
    message: SubscribeMessageType,
    ws: WebSocketWithId,
  ): void {
    logMessage("subscribe() is not implemented", LogMessageType.WARNING);
  }

  protected unsubscribe(
    message: UnsubscribeMessageType,
    ws: WebSocketWithId,
  ): void {
    logMessage("unsubscribe() is not implemented", LogMessageType.WARNING);
  }

  unsubscribe_client(ws: WebSocketWithId): void {
    logMessage(
      "unsubscribe_client() is not implemented",
      LogMessageType.WARNING,
    );
  }

  handleMessage(message: NewMessage, ws: WebSocketWithId): void {
    try {
      switch (message.type) {
        case "get":
          const aGetMessage = ensureMessageType<GetMessageType>(
            message,
            NewMessageType.Get,
          );
          void this.get(aGetMessage, ws);
          break;
        case "set":
          const aSetMessage = ensureMessageType<SetMessageType>(
            message,
            NewMessageType.Set,
          );
          this.set(aSetMessage, ws);
          break;
        case "subscribe":
          const aSubscribeMessage = ensureMessageType<SubscribeMessageType>(
            message,
            NewMessageType.Subscribe,
          );
          this.subscribe(aSubscribeMessage, ws);
          break;
        case "unsubscribe":
          const anUnsubscribeMessage =
            ensureMessageType<UnsubscribeMessageType>(
              message,
              NewMessageType.Unsubscribe,
            );
          this.unsubscribe(anUnsubscribeMessage, ws);
          break;
        default:
          throw new Error("Unknown message type.");
      }
    } catch (error: unknown) {
      const errMsg = error instanceof Error ? error.message : "Unknown error";
      this.sendMessage(
        ws,
        this.createErrorMessage(
          STATUS_ERRORS.INTERNAL_SERVER_ERROR,
          "Internal server error",
          errMsg,
          message.requestId,
        ),
      );
    }
  }

  /**
   * Takes the queryResults and sends a response to the client. 3 different responses are possible:
   * 1. Valid response with all available datapoints in the DB (number of datapoints can be less than requested)
   * 2. Client Error 404 if no requested datapoint could be found.
   * 3. Internal Server Error 500 if the DB access was not successful.
   */
  protected sendGetResponseToClient(
    queryResult: QueryResult,
    vin: string,
    requestedDataPoints: string[],
    ws: WebSocketWithId,
    requestId: string,
    path: string,
    root: "absolute" | "relative",
    format: "nested" | "flat",
  ): void {
    // There was an issue accessing the DB, notify the client and return
    if (!queryResult.success) {
      this.sendMessageToClient(
        ws,
        this.createErrorMessage(
          STATUS_ERRORS.INTERNAL_SERVER_ERROR,
          "Internal server error",
          queryResult.error,
          requestId,
        ),
      );
      return;
    }

    let responseMessage: DataContentMessage | ErrorMessage;
    if (queryResult.dataPoints.length > 0) {
      responseMessage = this.createDataContentMessage(
        vin,
        queryResult.dataPoints,
        root,
        format,
        path,
        queryResult.metadata,
        requestId,
      );
    } else {
      responseMessage = this.createErrorMessage(
        STATUS_ERRORS.NOT_FOUND,
        `Data not found`,
        `No values found for dataPoints [${replaceUnderscoresWithDots(requestedDataPoints.join(", "))}] for the instance: ${vin}`,
        requestId,
      );
    }

    this.sendMessageToClient(ws, responseMessage);
  }

  /**
   * Sends a message to the client.
   */
  protected sendMessageToClient(
    ws: WebSocketWithId,
    message: StatusMessage | DataContentMessage | ErrorMessage,
  ): void {
    this.sendMessage(ws, message);
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
    requestId: string,
  ): StatusMessage {
    return {
      type: "status",
      code: code,
      message: statusMessage,
      requestId: requestId,
      timestamp: getCurrentTimestamp(),
    };
  }

  /**
   * Generic function to create an error message.
   * @param code - status code (http-based).
   * @param message - A short message.
   * @param reason - A descriptive reason for the error.
   * @param requestId - The ID of the corresponding request.
   * @returns - A error message.
   */
  protected createErrorMessage(
    code: number,
    message: string,
    reason: string,
    requestId: string,
  ): ErrorMessage {
    return {
      type: "error",
      code: code,
      message: message,
      reason: reason,
      requestId: requestId,
    };
  }

  /**
   * Generic function to create a data content message.
   * @param instance - The ID of the element in the tree.
   * @param dataPoints - the data points with values.
   * @param root - "absolute" or "relative" paths.
   * @param format - "nested" or "flat" data structure.
   * @param path - The path associated with the data points.
   * @param metadata - available metadata for the data points
   * @param requestId - The ID of the corresponding request.
   * @returns - A data content message.
   */
  protected createDataContentMessage(
    instance: string,
    dataPoints: Array<{ name: string; value: any }>,
    root: "absolute" | "relative",
    format: "nested" | "flat",
    path: string,
    metadata?: Array<{ name: string; value: any }>,
    requestId?: string,
  ): DataContentMessage {
    // Ensure nodes are valid
    if (dataPoints.length === 0) {
      throw new Error("Nodes array cannot be empty.");
    }

    // Process root first: determine the base path for each datapoint
    // Then process format: structure the output as flat or nested
    const schema = getSchemaOrThrow(dataPoints);
    const requestedPath = normalizeRequestedPath(
      replaceUnderscoresWithDots(path),
      schema,
    );

    return {
      type: "data",
      instance: instance,
      schema: schema,
      data:
        format === "nested"
          ? buildDataStructureAsTree(dataPoints, root, requestedPath, schema)
          : buildDataStructureFlat(dataPoints, root, requestedPath, schema),
      requestId: requestId,
      ...(metadata &&
        Object.keys(metadata).length && {
          metadata: buildDataStructureFlat(
            metadata,
            root,
            requestedPath,
            schema,
          ),
        }),
    };
  }

  protected extractNodesFromData(
    path: string,
    data: Record<string, any>,
  ): Record<string, any> {
    const result: Record<string, any> = {};

    function traverse(
      currentPath: string,
      currentData: Record<string, any>,
    ): void {
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
    result: { [key: string]: any } = {},
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
    dataPointsSchema: DataPointSchema,
  ): string | null {
    // build the nodes from path and data
    if (message.type == NewMessageType.PermissionsEdit) {
      return null;
    }
    const data = message.type === NewMessageType.Set ? message.data : undefined;
    const requestNodes = extractLeafNodesFromRequest(message.path, data);

    const unknownFields = Array.from(requestNodes).filter((node) => {
      return !dataPointsSchema.hasOwnProperty(node);
    });

    if (unknownFields.length > 0) {
      return (
        "Could not find node: " +
        replaceUnderscoresWithDots(unknownFields.join(", "))
      );
    }
    return null;
  }

  protected extractNodesFromMessage(
    message: SetMessageType,
  ): Record<string, any> {
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

  protected extractNodesFromMessageWithVinAsNode(
    message: SetMessageType,
  ): Record<string, any> {
    const result = this.extractNodesFromMessage(message);
    // Add VIN as Node
    const { dataPointId } = databaseParams["VSS"];
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
    Array.from(leafNodes, (value) => replaceDotsWithUnderscore(value)),
  );
}

/**
 * Return the schema that is always the prefix (until the first dot) of all node names.
 * Throw in case of error
 * @param nodes list of nodes with names and values
 */
function getSchemaOrThrow(nodes: Array<{ name: string; value: any }>) {
  return (
    nodes[0]?.name.split(".")[0] ??
    (() => {
      throw new Error("Nodes array is empty");
    })()
  );
}

/**
 * Build flat data structure.
 * Process root first: determine base path relative to requested path.
 * Then format as flat: single-level key-value pairs.
 */
function buildDataStructureFlat(
  nodes: Array<{ name: string; value: any }>,
  root: "absolute" | "relative",
  requestedPath: string,
  schema: string,
): Record<string, any> {
  return nodes.reduce(
    (accumulator, { name, value }) => {
      let processedName: string;

      if (root === "relative") {
        // Remove everything up to and including the requested path
        // E.g., "Vehicle.CurrentLocation.Latitude" with path "CurrentLocation" -> "Latitude"
        // E.g., "Vehicle.Speed" with path "" -> "Speed"
        const fullPath = requestedPath ? `${schema}.${requestedPath}` : schema;
        if (name.startsWith(fullPath + ".")) {
          processedName = name.substring(fullPath.length + 1);
        } else if (name === fullPath) {
          processedName = ""; // Exact match - use empty string
        } else if (name.startsWith(schema + ".")) {
          // Fall back to removing the schema prefix
          processedName = name.substring(schema.length + 1);
        } else {
          processedName = name;
        }
      } else {
        // Absolute: keep full path excluding only the schema prefix
        // E.g., "Vehicle.CurrentLocation.Latitude" -> "CurrentLocation.Latitude"
        processedName = name.includes(".")
          ? name.split(".").slice(1).join(".")
          : name;
      }

      accumulator[processedName] = value;
      return accumulator;
    },
    {} as Record<string, any>,
  );
}

/**
 * Build nested data structure.
 * Process root first: determine base path relative to requested path.
 * Then format as nested: hierarchical object structure.
 */
function buildDataStructureAsTree(
  nodes: Array<{ name: string; value: any }>,
  root: "absolute" | "relative",
  requestedPath: string,
  schema: string,
): any {
  // First, process each node name based on root setting
  const processedNodes = nodes.map(({ name, value }) => {
    let processedName: string;

    if (root === "relative") {
      // Remove everything up to and including the requested path
      const fullPath = requestedPath ? `${schema}.${requestedPath}` : schema;
      if (name.startsWith(fullPath + ".")) {
        processedName = name.substring(fullPath.length + 1);
      } else if (name === fullPath) {
        processedName = ""; // Exact match - return value directly
      } else if (name.startsWith(schema + ".")) {
        // Fall back to removing the schema prefix
        processedName = name.substring(schema.length + 1);
      } else {
        processedName = name;
      }
    } else {
      // Absolute: keep full path excluding only the schema prefix
      processedName = name.includes(".")
        ? name.split(".").slice(1).join(".")
        : name;
    }

    return { name: processedName, value };
  });

  // Special case: if there's only one node with empty name, return the value directly
  if (processedNodes.length === 1 && processedNodes[0].name === "") {
    return processedNodes[0].value;
  }

  // Build nested structure from processed names
  const data: any = {};

  for (const { name, value } of processedNodes) {
    if (name === "") {
      // Empty path - this shouldn't happen in nested format with multiple nodes
      continue;
    }

    const pathParts = name.split(".");
    let currentLevel = data;

    for (let j = 0; j < pathParts.length; j++) {
      const key = pathParts[j];
      if (j === pathParts.length - 1) {
        // Leaf node
        currentLevel[key] = value;
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

function normalizeRequestedPath(path: string, schema: string): string {
  if (!path) {
    return "";
  }

  if (path === schema) {
    return "";
  }

  if (path.startsWith(schema + ".")) {
    return path.substring(schema.length + 1);
  }

  return path;
}

function getCurrentTimestamp(): { seconds: number; nanos: number } {
  const now = new Date();
  const seconds = Math.floor(now.getTime() / 1000); // Convert milliseconds to seconds
  const nanos = (now.getTime() % 1000) * 1e6; // Remainder in milliseconds converted to nanoseconds
  return { seconds, nanos: nanos };
}

export type QueryResult =
  | {
      success: true;
      dataPoints: Array<{ name: string; value: any }>;
      metadata: Array<{ name: string; value: any }>;
    }
  | { success: false; error: string };
