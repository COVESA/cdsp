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
import {replaceUnderscoresWithDots, toResponseFormat} from "../../../utils/transformations";
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
 * @param subscribedProperties -  Name of properties that values should be returned
 * @param mediaElement - The media element object with updated values.
 * @returns An array of objects, each containing the name and value of a subscribed property.
 */
function parseOnMediaElementChangeResponse(
  subscribedProperties: Set<string>,
  mediaElement: any
) {
  return Array.from(subscribedProperties).map((dataPointName) => ({
    name: replaceUnderscoresWithDots(dataPointName),
    value: mediaElement[dataPointName],
  }));
}

type Subscription = { listener: any, dataPoints: Set<string> };

/**
 * Handles the db connection for each client. Each client must get its own RealmDBHandler instance to work correctly.
 */
export class RealmDBHandler extends HandlerBase {
  private realm: Realm | null = null;
  private instanceToSubscriptionMap: Map<string, Subscription> = new Map();
  private mediaElementCache = new Map<string, Realm.Object>();

  constructor() {
    super();
    if (!databaseConfig) {
      throw new Error("Invalid database configuration.");
    }
  }

  getKnownDatapointsByPrefix(prefix: string) : string[] {
    const mediaElementSchema = this.realm?.schema.find(
      (schema) => schema.name === mediaElementsParams["VSS"].databaseName
    );
    const allSchemaFields: string[] = Object.keys(mediaElementSchema?.properties ?? {})
    // remove primary key and VIN from the list of fields, so only datapoints are left
    const allKnownDataPoints = allSchemaFields.filter(
      field => field !== PRIMARY_KEY && field !== mediaElementsParams["VSS"].dataPointId
    )

    if (allKnownDataPoints.length === 0) {
      logError("Retrieving known datapoints from RealmDB was not possible. This should not happen!", new Error())
    }
    return allKnownDataPoints.filter(value => value.startsWith(prefix));
  }

  async getDataPointsFromDB(dataPoints: string[], instance: string): Promise<QueryResult> {
    const mediaElement = await this.getMediaElement(instance);
    if (!mediaElement) {
      return {success: false, error: `No data found for instance ${instance}`}
    }

    logWithColor(`Media Element: \n ${JSON.stringify(mediaElement)}`, COLORS.GREY);
    const responseNodes = this.extractNodes(dataPoints, mediaElement);
    const responseNodesWithDots = responseNodes.map(node => ({
      name: replaceUnderscoresWithDots(node.name),
      value: node.value
    }));

    return {success: true, dataPoints: responseNodesWithDots, metadata: Array.of()};
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
    const newDataPoints = this.getKnownDatapointsByPrefix(message.path);
    if (newDataPoints.length === 0) {
      this.sendRequestedDataPointsNotFoundErrorMsg(ws, message.path)
      return;
    }

    let subscription = this.instanceToSubscriptionMap.get(message.instance) ?? {
      listener: undefined,
      dataPoints: new Set<string>()
    };

    const alreadySubscribed = newDataPoints.every((dataPoint: string) => subscription.dataPoints.has(dataPoint));
    if (alreadySubscribed) {
      this.sendAlreadySubscribedErrorMsg(ws, message.instance, newDataPoints)
      return;
    }

    // Update subscribed data points
    newDataPoints.forEach(dataPoint => subscription.dataPoints.add(dataPoint));

    // Update listener
    const responseMessage = await this.updateDBListenerAndCreateResponseMsg(message.instance, subscription, ws);
    this.sendMessageToClient(ws, responseMessage)
    this.logCurrentSubscriptions(ws.id);
  }

  protected async unsubscribe(message: UnsubscribeMessageType, ws: WebSocketWithId): Promise<void> {
    const dataPointsToUnsub = this.getKnownDatapointsByPrefix(message.path);
    if (dataPointsToUnsub.length === 0) {
      this.sendRequestedDataPointsNotFoundErrorMsg(ws, message.path)
      return;
    }

    let subscription = this.instanceToSubscriptionMap.get(message.instance);
    if (!subscription) {
      this.sendMessageToClient(ws, this.createStatusMessage(
        STATUS_ERRORS.NOT_FOUND,
        `Cannot unsubscribe. No subscription for instance '${message.instance}'`)
      );
      return;
    }

    const notSubscribed = dataPointsToUnsub.every(dataPoint => !subscription.dataPoints.has(dataPoint));
    if (notSubscribed) {
      this.sendMessageToClient(ws, this.createStatusMessage(
        STATUS_ERRORS.NOT_FOUND,
        `Cannot unsubscribe. No subscription for instance '${message.instance}' ` +
        `and datapoints [${toResponseFormat(dataPointsToUnsub)}].`
      ));
      return;
    }

    // Remove the data points from the subscription
    dataPointsToUnsub.forEach((dataPoint) => subscription.dataPoints.delete(dataPoint));

    let responseMessage;
    if (subscription.dataPoints.size > 0) {
      responseMessage = await this.updateDBListenerAndCreateResponseMsg(message.instance, subscription, ws);
    } else {
      responseMessage = await this.removeDBListenerAndCreateResponseMsg(message.instance, subscription)
    }

    this.sendMessageToClient(ws, responseMessage)
    this.logCurrentSubscriptions(ws.id)
  }

  async unsubscribe_client(ws: WebSocketWithId): Promise<void> {
    for (const [instance, subscription] of this.instanceToSubscriptionMap.entries()) {
      await this.removeDBListenerAndCreateResponseMsg(instance, subscription);
    }
    this.logCurrentSubscriptions(ws.id)
  }

  private async updateDBListenerAndCreateResponseMsg(instance: string, subscription: Subscription, ws: WebSocketWithId) {
    let responseMessage: StatusMessage;
    try {
      const mediaElement = await this.getMediaElement(instance);
      if (!mediaElement) {
        return this.createStatusMessage(STATUS_ERRORS.NOT_FOUND,
          `Instance ${instance} does not exist in Database.`);
      }

      // remove current listener if existent 
      if (subscription.listener) {
        mediaElement.removeListener(subscription.listener);
      }

      const newListener = this.createNewListener(subscription, instance, ws);
      subscription.listener = newListener;
      mediaElement.addListener(subscription.listener);

      // Store current subscription
      this.instanceToSubscriptionMap.set(instance, subscription)
      responseMessage = this.createStatusMessage(STATUS_SUCCESS.OK, "Subscription changed successfully.");

    } catch (error: unknown) {
      const errMsg = error instanceof Error ? error.message : "Unknown error";
      return this.createStatusMessage(STATUS_ERRORS.SERVICE_UNAVAILABLE,
        `Subscription process could not finish, try again: ${errMsg}`);
    }
    return responseMessage;
  }

  private async removeDBListenerAndCreateResponseMsg(instance: string, subscription: Subscription) {
    const mediaElement = await this.getMediaElement(instance);
    if (!mediaElement) {
      return this.createStatusMessage(STATUS_ERRORS.NOT_FOUND,
        `Instance ${instance} does not exist in Database.`);
    }

    // remove current listener if existent 
    if (subscription.listener) {
      mediaElement.removeListener(subscription.listener);
    }

    this.instanceToSubscriptionMap.delete(instance);

    return this.createStatusMessage(STATUS_SUCCESS.OK, "Subscription changed successfully.");
  }

  /**
   * Create a new listener that notifies the client if any subscribed data points are changed.
   */
  private createNewListener(subscription: Subscription, instance: string, ws: WebSocketWithId) {
    return (mediaElement: any, changes: Changes) => {
      const changedSubscribedDatapoints = changes.changedProperties.filter(field => subscription.dataPoints.has(field));
      if (changes.deleted) {
        logWithColor(`Media element for ${instance} was deleted`, COLORS.YELLOW);
      } else if (changedSubscribedDatapoints.length > 0) {
        logMessage(`Media element for ${instance} changed`, LogMessageType.RECEIVED, `Web-Socket Connection Event Received`);
        this.notifyClientOfDataPointChanges(
          mediaElement,
          instance,
          ws,
          subscription.dataPoints
        );
      }
    };
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

  /**
   * Extract from the media element the requested datapoints with their values
   *
   * @param requestedDataPoints DataPoints of interest.
   * @param mediaElement - The RealmDB query response object containing all key value pairs.
   * @returns - A list of nodes (name-value pairs) from a media element for provided list of datapoints.
   * @private
   */
  private extractNodes(requestedDataPoints: string[], mediaElement: any): { name: string; value: any }[] {
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
   * Using a cache ensures that on consecutive calls the same media element reference is returned.
   *
   * @param vin - VIN
   * @returns - The media element object from the database for provided VIN
   * @private
   */
  private async getMediaElement(vin: string): Promise<Realm.Object | undefined> {
    try {
      if (this.mediaElementCache.has(vin)) {
        return this.mediaElementCache.get(vin);
      }

      const {databaseName, dataPointId} = mediaElementsParams["VSS"];
      const mediaElement = this.realm?.objects(databaseName)
        .filtered(`${dataPointId} = '${vin}'`)[0];

      if (mediaElement) {
        this.mediaElementCache.set(vin, mediaElement);
      }
      return mediaElement;

    } catch (error: unknown) {
      logError("Error trying to get media element from Realm", error);
    }
  }

  /**
   * Handles changes to a media element and sends update messages to clients.
   * @param mediaElement - The media element that has changed.
   * @param instance - Vehicle identifier for the client message.
   * @param ws - The WebSocket object for communication.
   * @param subscribedDataPoints
   */
  private notifyClientOfDataPointChanges(
    mediaElement: any, instance: string, ws: WebSocketWithId, subscribedDataPoints: Set<string>
  ): void {

    const responseNodes = parseOnMediaElementChangeResponse(
      subscribedDataPoints,
      mediaElement
    );
    const dataContentMessage = this.createDataContentMessage(instance, responseNodes, Array.of());
    this.sendMessageToClient(ws, dataContentMessage);
  }

  private logCurrentSubscriptions(clientID: string) {
    let logString = `Subscriptions of client ${clientID}:\n`;
    for (const [instance, sub] of this.instanceToSubscriptionMap.entries()) {
      logString += `${instance} =>  [${Array.from(sub.dataPoints).join(", ")}] \n`;
    }
    logWithColor(logString, COLORS.ORANGE);
  }
}
