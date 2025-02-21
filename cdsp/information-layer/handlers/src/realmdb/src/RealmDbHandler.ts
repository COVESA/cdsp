import Realm from "realm";
import {v4 as uuidv4} from "uuid";
import {HandlerBase, QueryResult} from "../../HandlerBase";
import {mediaElementsParams, databaseConfig, PRIMARY_KEY} from "../config/database-params";
import {realmConfig, SupportedDataPoints} from "../config/realm-config";
import {
  logMessage,
  logError,
  logWithColor,
  LogMessageType,
  COLORS,
  logErrorStr,
} from "../../../../utils/logger";
import {WebSocketWithId} from "../../../../utils/database-params";
import {STATUS_ERRORS, STATUS_SUCCESS} from "../../../../router/utils/ErrorMessage";
import {replaceUnderscoresWithDots} from "../../../utils/transformations";
import {
  NewMessage,
  SetMessageType,
  StatusMessage,
  SubscribeMessageType,
  UnsubscribeMessageType
} from "../../../../router/utils/NewMessage";

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
    name: replaceUnderscoresWithDots(prop),
    value: mediaElement[prop],
  }));
}

export class RealmDBHandler extends HandlerBase {
  private realm: Realm | null;
  private listeners: Map<WebSocketWithId, Map<string, any>>;

  constructor() {
    super();
    if (!databaseConfig) {
      throw new Error("Invalid database configuration.");
    }
    this.realm = null;
    this.listeners = new Map();
  }

  async authenticateAndConnect(): Promise<void> {
    try {
      const app = new Realm.App({id: databaseConfig!.realmAppId});
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

  getKnownDatapointsByPrefix(prefix: string) {
    const mediaElementSchema = this.realm?.schema.find(
      (schema) => schema.name === mediaElementsParams["VSS"].databaseName
    );
    const allSchemaFields: string[] = Object.keys(mediaElementSchema?.properties ?? {})
    // remove primary key from the list of fields, so only datapoints are left
    const allKnownDataPoints = allSchemaFields.filter(field => field !== PRIMARY_KEY)

    if (allKnownDataPoints.length === 0) {
      logError("Retrieving known datapoints from RealmDB was not possible. This should not happen!", new Error())
    }
    return allKnownDataPoints.filter(value => value.startsWith(prefix));
  }

  protected async set(message: SetMessageType, ws: WebSocketWithId): Promise<void> {
    if (this.areNodesValid(message, ws)) {
      try {
        const mediaElement = await this.getMediaElement(message.instance);
        const nodes = this.extractNodesFromMessage(message);

        const transformAndAssign = (element: any, nodes: Record<string, any>) => {
          Object.entries(nodes).forEach(([name, value]) => {
            element[name] = value;
          });
        };

        this.realm?.write(() => {
          if (mediaElement) {
            transformAndAssign(mediaElement, nodes ?? []);
          } else {
            if (!mediaElementsParams["VSS"]) {
              const errorMessage = "Tree does not exist in mediaElementsParams";
              logErrorStr(errorMessage);
              const statusMessage = this.createStatusMessage(STATUS_ERRORS.NOT_FOUND, errorMessage);
              this.sendMessageToClient(ws, statusMessage);
              return;
            }

            const dataPointId = mediaElementsParams["VSS"].dataPointId;
            const document = {_id: uuidv4(), [dataPointId]: message.instance};
            transformAndAssign(document, nodes ?? []);
            const databaseName = mediaElementsParams["VSS"].databaseName;
            this.realm?.create(databaseName, document);
          }
        });

        const statusMessage = this.createStatusMessage(STATUS_SUCCESS.OK, "Successfully wrote data to database");
        this.sendMessageToClient(ws, statusMessage);
      } catch (error: unknown) {
        const errMsg = error instanceof Error ? error.message : "Unknown error";
        const errorMessage = `Schema is not compatible for that media element: ${errMsg}`;
        logErrorStr(errorMessage);
        const statusMessage = this.createStatusMessage(STATUS_ERRORS.NOT_FOUND, errorMessage);
        this.sendMessageToClient(ws, statusMessage);
      }
    }
  }

  protected async subscribe(message: SubscribeMessageType, ws: WebSocketWithId): Promise<void> {
    let responseMessage: StatusMessage;
    try {
      const mediaElement = await this.getMediaElement(message.instance);

      if (mediaElement) {
        const objectId = mediaElement._id;
        if (!mediaElementsParams["VSS"]) {
          const errorMessage = "Tree does not exist in mediaElementsParams";
          logErrorStr(errorMessage);
          const statusMessage = this.createStatusMessage(STATUS_ERRORS.NOT_FOUND, errorMessage);
          this.sendMessageToClient(ws, statusMessage);
          return;
        }

        const {databaseName, dataPointId} = mediaElementsParams["VSS"];

        if (!this.listeners.has(ws)) {
          this.listeners.set(ws, new Map());
        }

        if (!this.listeners.get(ws)?.has(message.instance)) {
          logWithColor(
            `Subscribing element: Object ID: ${objectId} with ${dataPointId}: '${message.instance}' on ${databaseName}`,
            COLORS.GREY
          );

          const listener = (mediaElement: any, changes: Changes) =>
            this.onMediaElementChange(
              mediaElement,
              changes,
              message.instance,
              ws
            );

          mediaElement.addListener(listener);

          this.listeners.get(ws)?.set(message.instance, {
            objectId: objectId,
            mediaElement: mediaElement,
            listener: listener,
          });

          responseMessage = this.createStatusMessage(STATUS_SUCCESS.OK, "Successfully subscribed");

          logWithColor(
            `Subscription added! Amount Clients: ${this.listeners.size}`,
            COLORS.GREY
          );
        } else {
          responseMessage = this.createStatusMessage(STATUS_ERRORS.SERVICE_UNAVAILABLE,
            `Subscription already done to ${dataPointId}: '${message.instance}'`);
        }
      } else {
        responseMessage = this.createStatusMessage(STATUS_ERRORS.SERVICE_UNAVAILABLE,
          "Object not found");
      }
    } catch (error: unknown) {
      const errMsg = error instanceof Error ? error.message : "Unknown error";
      responseMessage = this.createStatusMessage(STATUS_ERRORS.SERVICE_UNAVAILABLE,
        `Subscription process could not finish, try again: ${errMsg}`);
    }
    this.sendMessageToClient(ws, responseMessage)
  }

  protected async unsubscribe(message: UnsubscribeMessageType, ws: WebSocketWithId): Promise<void> {
    if (!mediaElementsParams["VSS"]) {
      const errorMessage = "Tree does not exist in mediaElementsParams";
      logErrorStr(errorMessage);
      const statusMessage = this.createStatusMessage(STATUS_ERRORS.NOT_FOUND, errorMessage);
      this.sendMessageToClient(ws, statusMessage);
      return;
    }

    const {databaseName, dataPointId} = mediaElementsParams["VSS"];

    let responseMessage: StatusMessage;
    if (this.listeners.has(ws)) {
      const wsListeners = this.listeners.get(ws);
      if (wsListeners?.has(message.instance)) {
        const listener = wsListeners.get(message.instance);
        logWithColor(
          `Unsubscribing element: Object ID: ${listener.objectId} with ${dataPointId}: '${message.instance}' on ${databaseName}`,
          COLORS.GREY
        );
        listener.mediaElement.removeListener(listener.listener);
        wsListeners.delete(message.instance);

        if (wsListeners.size === 0) {
          this.listeners.delete(ws);
        }

        logWithColor(`Subscription removed! Amount Clients: ${this.listeners.size}`, COLORS.GREY);
        responseMessage = this.createStatusMessage(STATUS_SUCCESS.OK, "Successfully unsubscribed");
      } else {
        responseMessage = this.createStatusMessage(STATUS_ERRORS.BAD_REQUEST,
          `No subscription found for VIN: ${message.instance}`);
      }
    } else {
      responseMessage = this.createStatusMessage(STATUS_ERRORS.BAD_REQUEST,
        `No subscription found for this client`);
    }
    this.sendMessageToClient(ws, responseMessage);
  }

  async unsubscribe_client(ws: WebSocketWithId): Promise<void> {
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
  private areNodesValid(message: NewMessage, ws: WebSocketWithId): boolean {
    const {databaseName} = mediaElementsParams["VSS"];

    const mediaElementSchema = this.realm?.schema.find(
      (schema) => schema.name === databaseName
    );

    const errorMessage = this.validateNodesAgainstSchema(
      message,
      mediaElementSchema?.properties ?? {}
    );

    if (errorMessage) {
      logErrorStr(`Error validating message nodes against schema: ${errorMessage}`);
      const statusMessage = this.createStatusMessage(STATUS_ERRORS.NOT_FOUND, errorMessage);
      this.sendMessageToClient(ws, statusMessage);
      return false;
    }
    return true;
  }

  async getDataPointsFromDB(dataPoints: string[], vin: string): Promise<QueryResult> {
    const mediaElement = await this.getMediaElement(vin);
    if (!mediaElement) {
      return {success: false, error: `No data found with the Id: ${vin}`}
    }

    logWithColor(`Media Element: \n ${JSON.stringify(mediaElement)}`, COLORS.GREY);
    const responseNodes = this.extractNodes(dataPoints, mediaElement);
    const responseNodesWithDots = responseNodes.map(node => ({
      name: replaceUnderscoresWithDots(node.name),
      value: node.value
    }));

    return {success: true, nodes: responseNodesWithDots};
  }


  /**
   * Extract from the media element the requested datapoints with their values
   *
   * @param requestedDataPoints DataPoints of interest.
   * @param mediaElement - The RealmDB query response object containing all key value pairs.
   * @returns - A list of nodes (name-value pairs) from a media element for provided list of datapoints.
   * @private
   */
  private extractNodes(
    requestedDataPoints: string[],
    mediaElement: any
  ): { name: string; value: any }[] {
    const data: { name: string; value: any }[] = [];
    requestedDataPoints.forEach((dataPoint) => {
      data.push({
        name: dataPoint,
        value: mediaElement[dataPoint],
      });
    })
    return data;
  }

  /**
   * Asynchronously retrieves a media element from the database based on the provided vin.
   *
   * @param vin - VIN
   * @returns - The media element object from the database for provided VIN
   * @private
   */
  private async getMediaElement(vin: string): Promise<any> {
    try {
      const {databaseName, dataPointId} = mediaElementsParams["VSS"];
      return this.realm?.objects(databaseName)
        .filtered(`${dataPointId} = '${vin}'`)[0];
    } catch (error: unknown) {
      logError("Error trying to get media element from Realm", error);
    }
  }

  /**
   * Handles changes to a media element and sends update messages to clients.
   * @param mediaElement - The media element that has changed.
   * @param changes - An object containing information about the changes.
   * @param instance - Vehicle identifier for the client message.
   * @param ws - The WebSocket object for communication.
   */
  private onMediaElementChange(
    mediaElement: any,
    changes: Changes,
    instance: string,
    ws: WebSocketWithId
  ): void {
    logMessage(
      "Media element changed",
      LogMessageType.RECEIVED,
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
        const dataContentMessage = this.createDataContentMessage(instance, responseNodes);
        this.sendMessageToClient(ws, dataContentMessage);
      }
    }
  }
}
