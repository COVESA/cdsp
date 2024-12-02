import Realm from "realm";
import { v4 as uuidv4 } from "uuid";
import { HandlerBase } from "../../../../handlers/src/HandlerBase";
import { mediaElementsParams, databaseConfig } from "../config/database-params";
import { realmConfig, SupportedDataPoints } from "../config/realm-config";
import {
  logMessage,
  logError,
  logWithColor,
  MessageType,
  COLORS,
  logErrorStr,
} from "../../../../utils/logger";
import { createErrorMessage } from "../../../../utils/error-message-helper";
import { WebSocket, Message, STATUS_ERRORS } from "../../../utils/data-types";
import { transformDataPointsWithDots, transformDataPointsWithUnderscores } from "../../../utils/transformations";

// Define a type for changes
interface Changes {
  deleted: boolean;
  changedProperties: string[];
}

/**
 * Parses the response from a media element change event.
 *
 * @param changes - The object containing the changed properties.
 * @param mediaElement - The media element object with updated properties.
 * @returns An array of objects, each containing the name and value of a changed property.
 */
function parseOnMediaElementChangeResponse(
  changes: Changes,
  mediaElement: any
) {
  return changes.changedProperties.map((prop) => ({
    name: transformDataPointsWithDots(prop),
    value: mediaElement[prop],
  }));
}

export class RealmDBHandler extends HandlerBase {
  private realm: Realm | null;
  private sendMessageToClients:
    | ((ws: WebSocket, message: Message) => void)
    | null;
  private listeners: Map<WebSocket, Map<string, any>>;

  constructor() {
    super();
    if (!databaseConfig) {
      throw new Error("Invalid database configuration.");
    }
    this.realm = null;
    this.sendMessageToClients = null;
    this.listeners = new Map();
  }

  async authenticateAndConnect(
    sendMessageToClients: (ws: WebSocket, message: Message) => void
  ): Promise<void> {
    try {
      this.sendMessageToClients = sendMessageToClients;

      const app = new Realm.App({ id: databaseConfig!.realmAppId });
      const credentials = Realm.Credentials.apiKey(databaseConfig!.realmApiKey);
      const user = await app.logIn(credentials);
      logMessage("Successfully authenticated to RealmDB");

      const supportedDataPoints =
        this.getSupportedDataPoints() as SupportedDataPoints;
      const realmConfigObj = realmConfig(user, supportedDataPoints);
      this.realm = await Realm.open(realmConfigObj);
      logMessage("Connection established successfully");

      for (const [key, value] of Object.entries(mediaElementsParams)) {
        try {
          const databaseName = value.databaseName;
          await this.realm.objects(databaseName).subscribe();
          logMessage(`Subscribed to the database ${key}: ${databaseName}`);
        } catch (error: unknown) {
          logError("Error subscribing databases", error);
        }
      }
    } catch (error: unknown) {
      logError("Failed to authenticate with Realm", error);
    }
  }

  protected async read(message: Message, ws: WebSocket): Promise<void> {
    if (this.areNodesValid(message, ws)) {
      try {
        const updateMessage = await this.getMessageData(message);
        this.sendMessageToClient(ws, updateMessage);
      } catch (error: unknown) {
        const errMsg = error instanceof Error ? error.message : "Unknown error";
        logError("Error reading object from Realm", error);
        this.sendMessageToClient(
          ws,
          createErrorMessage("read", STATUS_ERRORS.NOT_FOUND, errMsg)
        );
      }
    }
  }

  protected async write(message: Message, ws: WebSocket): Promise<void> {
    if (this.areNodesValid(message, ws)) {
      try {
        const mediaElement = await this.getMediaElement(message);
        const nodes = message.node ? [message.node] : message.nodes;

        const transformAndAssign = (element: any, nodes: any[]) => {
          nodes.forEach(({ name, value }) => {
            const prop = transformDataPointsWithUnderscores(name);
            element[prop] = value;
          });
        };

        this.realm?.write(() => {
          if (mediaElement) {
            transformAndAssign(mediaElement, nodes ?? []);
          } else {
            if (!message.tree || !mediaElementsParams[message.tree]) {
              const errorMessage =
                "Tree is undefined or does not exist in mediaElementsParams";
              logErrorStr(errorMessage);
              this.sendMessageToClient(
                ws,
                createErrorMessage(
                  "write",
                  STATUS_ERRORS.NOT_FOUND,
                  errorMessage
                )
              );
              return;
            }

            const dataPointId = mediaElementsParams[message.tree].dataPointId;
            const document = { _id: uuidv4(), [dataPointId]: message.id };
            transformAndAssign(document, nodes ?? []);
            const databaseName = mediaElementsParams[message.tree].databaseName;
            this.realm?.create(databaseName, document);
          }
        });

        await this.read(message, ws);
      } catch (error: unknown) {
        const errMsg = error instanceof Error ? error.message : "Unknown error";
        const errorMessage = `Schema is not compatible for that media element: ${errMsg}`;
        logErrorStr(errorMessage);
        this.sendMessageToClient(
          ws,
          createErrorMessage("write", STATUS_ERRORS.NOT_FOUND, errorMessage)
        );
      }
    }
  }

  protected async subscribe(message: Message, ws: WebSocket): Promise<void> {
    try {
      const mediaElement = await this.getMediaElement(message);

      if (mediaElement) {
        const objectId = mediaElement._id;
        const { id, tree, uuid } = message;
        if (!id || !tree || !mediaElementsParams[tree]) {
          const errorMessage =
            "Tree or id is undefined or does not exist in mediaElementsParams";
          logErrorStr(errorMessage);
          this.sendMessageToClient(
            ws,
            createErrorMessage("write", STATUS_ERRORS.NOT_FOUND, errorMessage)
          );
          return;
        }

        const { databaseName, dataPointId } = mediaElementsParams[tree];

        if (!this.listeners.has(ws)) {
          this.listeners.set(ws, new Map());
        }

        if (!this.listeners.get(ws)?.has(id)) {
          logWithColor(
            `Subscribing element for user '${uuid}': Object ID: ${objectId} with ${dataPointId}: '${id}' on ${databaseName}`,
            COLORS.GREY
          );

          const listener = (mediaElement: any, changes: Changes) =>
            this.onMediaElementChange(
              mediaElement,
              changes,
              { id, tree, uuid },
              ws
            );

          mediaElement.addListener(listener);

          this.listeners.get(ws)?.set(id, {
            objectId: objectId,
            mediaElement: mediaElement,
            listener: listener,
          });

          this.sendMessageToClient(
            ws,
            this.createSubscribeStatusMessage("subscribe", message, "succeed")
          );

          logWithColor(
            `Subscription added! Amount Clients: ${this.listeners.size}`,
            COLORS.GREY
          );
        } else {
          this.sendMessageToClient(
            ws,
            createErrorMessage(
              "subscribe",
              STATUS_ERRORS.BAD_REQUEST,
              `Subscription already done to ${dataPointId}: '${id}'`
            )
          );
        }
      } else {
        this.sendMessageToClient(
          ws,
          createErrorMessage(
            "subscribe",
            STATUS_ERRORS.BAD_REQUEST,
            "Object not found"
          )
        );
      }
    } catch (error: unknown) {
      const errMsg = error instanceof Error ? error.message : "Unknown error";
      this.sendMessageToClient(
        ws,
        createErrorMessage(
          "subscribe",
          STATUS_ERRORS.SERVICE_UNAVAILABLE,
          `Subscription process could not finish, try again: ${errMsg}`
        )
      );
    }
  }

  protected async unsubscribe(message: Message, ws: WebSocket): Promise<void> {
    const { id, tree, uuid } = message;
    if (!id || !tree || !mediaElementsParams[tree]) {
      const errorMessage =
        "Tree or id is undefined or does not exist in mediaElementsParams";
      logErrorStr(errorMessage);
      this.sendMessageToClient(
        ws,
        createErrorMessage("write", STATUS_ERRORS.NOT_FOUND, errorMessage)
      );
      return;
    }

    const { databaseName, dataPointId } = mediaElementsParams[tree];

    if (this.listeners.has(ws)) {
      const wsListeners = this.listeners.get(ws);
      if (wsListeners?.has(id)) {
        const listener = wsListeners.get(id);
        logWithColor(
          `Unsubscribing element for user '${uuid}': Object ID: ${listener.objectId} with ${dataPointId}: '${id}' on ${databaseName}`,
          COLORS.GREY
        );
        listener.mediaElement.removeListener(listener.listener);
        wsListeners.delete(id);

        if (wsListeners.size === 0) {
          this.listeners.delete(ws);
        }

        this.sendMessageToClient(
          ws,
          this.createSubscribeStatusMessage("unsubscribe", message, "succeed")
        );
      } else {
        this.sendMessageToClient(
          ws,
          createErrorMessage(
            "unsubscribe",
            STATUS_ERRORS.BAD_REQUEST,
            `No subscription found for VIN: ${id}`
          )
        );
      }
    } else {
      this.sendMessageToClient(
        ws,
        createErrorMessage(
          "unsubscribe",
          STATUS_ERRORS.BAD_REQUEST,
          `No subscription found for this client`
        )
      );
    }
    logWithColor(
      `Subscription removed! Amount Clients: ${this.listeners.size}`,
      COLORS.GREY
    );
  }

  async unsubscribe_client(ws: WebSocket): Promise<void> {
    this.listeners.delete(ws);
    logWithColor(
      `All client subscriptions removed! Amount Clients: ${this.listeners.size}`,
      COLORS.GREY
    );
  }

  /**
   * Validates the nodes in a message against the schema of a media element.
   *
   * @param message - The message object containing details for the request.
   * @param ws - The WebSocket object for communication.
   * @returns - Returns true if all nodes are valid against the schema, otherwise false.
   */
  private areNodesValid(message: Message, ws: WebSocket): boolean {
    const { type, tree } = message;
    if (!tree || !mediaElementsParams[tree]) {
      logErrorStr("Tree is undefined or does not exist in mediaElementsParams");
      return false;
    }

    const { databaseName } = mediaElementsParams[tree];

    const mediaElementSchema = this.realm?.schema.find(
      (schema) => schema.name === databaseName
    );

    const errorData = this.validateNodesAgainstSchema(
      message,
      mediaElementSchema?.properties ?? {}
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
   * Asynchronously processes a message to fetch and handle media data.
   *
   * @param  message - The message object containing details for the request.
   * @returns Returns a promise that resolves to an updated message object.
   * @throws Throws an error if no media element for the message ID is found.
   * @private
   */
  private async getMessageData(message: Message): Promise<Message> {
    const mediaElement = await this.getMediaElement(message);

    if (!mediaElement) {
      throw new Error(`No data found with the Id: ${message.id}`);
    }

    logWithColor(
      `Media Element: \n ${JSON.stringify(mediaElement)}`,
      COLORS.GREY
    );

    const responseNodes = this.parseReadResponse(message, mediaElement);
    return this.createUpdateMessage(message.id, message.tree, message.uuid, responseNodes);
  }

  /**
   * Parses the response from a read event.
   *
   * @param message - The message object containing node or nodes information.
   * @param queryResponseObj - The query response object containing values to be mapped.
   * @returns - A data object with keys from the message nodes and values from the query response.
   * @private
   */
  private parseReadResponse(
    message: Message,
    queryResponseObj: any
  ): { name: string; value: any }[] {
    const data: { name: string; value: any }[] = [];
    const nodes = message.node ? [message.node] : message.nodes;
    nodes?.forEach((node: any) => {
      const prop = transformDataPointsWithUnderscores(node.name);
      data.push({
        name: node.name,
        value: queryResponseObj[prop],
      });
    });
    return data;
  }

  /**
   * Asynchronously retrieves a media element from the database based on the provided message.
   *
   * @param message - The message containing the id and tree information.
   * @returns - The media element object from the database.
   * @private
   */
  private async getMediaElement(message: Message): Promise<any> {
    try {
      const { id, tree } = message;
      if (!tree || !mediaElementsParams[tree]) {
        logErrorStr(
          "Tree is undefined or does not exist in mediaElementsParams"
        );
        return false;
      }

      const { databaseName, dataPointId } = mediaElementsParams[tree];
      return await this.realm
        ?.objects(databaseName)
        .filtered(`${dataPointId} = '${id}'`)[0];
    } catch (error: unknown) {
      logError("Error trying to get media element from Realm", error);
    }
  }

  /**
   * Handles changes to a media element and sends update messages to clients.
   * @param mediaElement - The media element that has changed.
   * @param changes - An object containing information about the changes.
   * @param messageHeader - The header information for the message.
   * @param ws - The WebSocket object for communication.
   */
  private onMediaElementChange(
    mediaElement: any,
    changes: Changes,
    messageHeader: Pick<Message, "id" | "tree" | "uuid">,
    ws: WebSocket
  ): void {
    logMessage(
      "Media element changed",
      MessageType.RECEIVED,
      `Web-Socket Connection Event Received`
    );
    if (changes.deleted) {
      logWithColor("MediaElement is deleted", COLORS.YELLOW);
    } else {
      if (changes.changedProperties.length > 0) {
        const responseNodes = parseOnMediaElementChangeResponse(
          changes,
          mediaElement
        );
        const updateMessage = this.createUpdateMessage(
          messageHeader.id, messageHeader.tree, messageHeader.uuid,
          responseNodes
        );
        this.sendMessageToClient(ws, updateMessage);
      }
    }
  }
}
