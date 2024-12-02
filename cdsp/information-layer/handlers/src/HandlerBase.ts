import fs from "fs";
import yaml from "js-yaml";
import {
  Message,
  WebSocket,
  WebSocketWithId,
  DataPointSchema,
  MessageBase,
  ErrorMessage,
} from "../utils/data-types";
import { getDataPointsPath } from "../config/config";
import { logMessage, MessageType } from "../../utils/logger";
import { transformDataPointsWithUnderscores } from "../utils/transformations";


export abstract class HandlerBase {
  // Default implementations of required functions
  authenticateAndConnect(sendMessageToClients: (message: any) => void): void {
    logMessage(
      "authenticateAndConnect() is not implemented",
      MessageType.WARNING
    );
  }

  protected read(message: Message, ws: WebSocket): void {
    logMessage("read() is not implemented", MessageType.WARNING);
  }

  protected write(message: Message, ws: WebSocket): void {
    logMessage("write() is not implemented", MessageType.WARNING);
  }

  protected subscribe(message: Message, ws: WebSocket): void {
    logMessage("subscribe() is not implemented", MessageType.WARNING);
  }

  protected unsubscribe(message: Message, ws: WebSocket): void {
    logMessage("unsubscribe() is not implemented", MessageType.WARNING);
  }

  unsubscribe_client(ws: WebSocket): void {
    logMessage("unsubscribe_client() is not implemented", MessageType.WARNING);
  }

  handleMessage(message: Message, connectionId: string, ws: WebSocketWithId): void {
    try {
      switch (message.type) {
        case "read":
          this.read(message, ws);
          break;
        case "write":
          this.write(message, ws);
          break;
        case "subscribe":
          this.subscribe(message, ws);
          break;
        case "unsubscribe":
          this.unsubscribe(message, ws);
          break;
        default:
          ws.send(JSON.stringify({ error: "Unknown message type" }));
      }
    } catch (error: unknown) {
      const errMsg = error instanceof Error ? error.message : "Unknown error";
      ws.send(errMsg);
    }
  }

  /**
   * Sends a message to the client.
   */
  protected sendMessageToClient(
    ws: WebSocket,
    message: Message | MessageBase | ErrorMessage
  ): void {
    logMessage(JSON.stringify(message), MessageType.SENT);
    ws.send(JSON.stringify(message));
  }

  /**
   * Generic function to create an update message.
   * @param message - The original message from client.
   * @param nodes - The nodes to be included in the message.
   * @returns - The transformed message.
   */
  protected createUpdateMessage(
    id: string, tree: string, uuid: string,
    nodes: Array<{ name: string; value: any }>
  ): Message {
    return {
      type: "update",
      tree,
      id,
      dateTime: new Date().toISOString(),
      uuid,
      ...(nodes.length === 1
        ? { node: nodes[0] } // Return single node as 'node'
        : { nodes }), // Return array as 'nodes' } as Message;
    };
  }

  /**
   * Generic function to create a subscription status message.
   * @param type - Type of subscription message.
   * @param message - The original message from client.
   * @param status - The status of the subscription.
   * @returns - The transformed message.
   */
  protected createSubscribeStatusMessage(
    type: "subscribe" | "unsubscribe",
    message: Pick<Message, "id" | "tree" | "uuid">,
    status: string
  ): MessageBase {
    const { id, tree, uuid } = message;
    return {
      type: `${type}:status` as MessageBase["type"],
      tree,
      id,
      dateTime: new Date().toISOString(),
      uuid,
      status: status,
    };
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
      const underscored_node = transformDataPointsWithUnderscores(node);
      if (value !== null) {
        result[underscored_node] = value;
      }
    });
    return result;
  }

  /**
   * Validates nodes against a given schema.
   *
   * @param message - The message containing nodes to be validated.
   * @param dataPointsSchema - The schema against which nodes are validated.
   * @returns An object containing error details if validation fails, otherwise null.
   */
  protected validateNodesAgainstSchema(
    message: Message,
    dataPointsSchema: DataPointSchema
  ): object | null {
    const nodes = message.node ? [message.node] : message.nodes || [];

    const unknownFields = nodes.filter(({ name }) => {
      const transformedName = transformDataPointsWithUnderscores(name);
      return !dataPointsSchema.hasOwnProperty(transformedName);
    });

    if (unknownFields.length > 0) {
      const errors = unknownFields.map(({ name }) => ({
        name,
        status: "Parent object or node not found.",
      }));
      return errors.length === 1 ? { node: errors[0] } : { nodes: errors };
    }
    return null;
  }
}
