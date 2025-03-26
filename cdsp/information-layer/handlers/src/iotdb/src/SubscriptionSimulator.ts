import {logMessage, logError, logErrorStr} from "../../../../utils/logger";
import {databaseConfig, databaseParams} from "../config/database-params";
import {Session} from "./Session";
import {SessionDataSet} from "../utils/SessionDataSet";
import {WebSocketWithId} from "../../../../utils/database-params";
import {
  DataContentMessage,
  StatusMessage,
  SubscribeMessageType,
  UnsubscribeMessageType
} from "../../../../router/utils/NewMessage";
import {ErrorMessage, STATUS_ERRORS, STATUS_SUCCESS} from "../../../../router/utils/ErrorMessage";
import {transformSessionDataSet} from "../utils/database-helper";
import {toResponseFormat} from "../../../utils/transformations";
import {METADATA_SUFFIX} from "../utils/iotdb-constants";

export type Subscription = { vin: string, dataPoints: Set<string> }
export type WebsocketToSubscriptionsMap = Map<WebSocketWithId, Subscription[]>;
const TREE_VSS = "VSS";

// Define the singleton instance at the module level
let subscriptionSimulatorInstance: SubscriptionSimulator | null = null;

// Function to get or initialize the singleton instance
export function getSubscriptionSimulator(
  sendMessageToClient: (ws: WebSocketWithId, message: StatusMessage | DataContentMessage | ErrorMessage) => void,
  createDataContentMessage: (instance: string,
                             dataPoints: Array<{ name: string; value: any; }>,
                             metadata: Array<{ name: string; value: any; }>,
                             requestId?: string) => DataContentMessage,
  createStatusMessage: (code: number, statusMessage: string, requestId?: string) => StatusMessage,
  sendAlreadySubscribedErrorMsg: (ws: WebSocketWithId, vin: string, newDataPoints: string[]) => void)
  : SubscriptionSimulator {
  if (!subscriptionSimulatorInstance) {
    subscriptionSimulatorInstance = new SubscriptionSimulator(
      sendMessageToClient,
      createDataContentMessage,
      createStatusMessage,
      sendAlreadySubscribedErrorMsg
    );
    logMessage("SubscriptionSimulator instance created.");
  }
  return subscriptionSimulatorInstance;
}

export class SubscriptionSimulator {
  private intervalId: NodeJS.Timeout | null = null;
  private session: Session;
  private timeIntervalLowerLimit: number | undefined = undefined;
  private websocketToSubscriptionsMap: WebsocketToSubscriptionsMap = new Map();
  private readonly sendMessageToClient: (ws: WebSocketWithId, message: StatusMessage | DataContentMessage | ErrorMessage) => void;
  private readonly createDataContentMessage: (
    instance: string,
    dataPoints: Array<{ name: string; value: any }>,
    metadata: Array<{ name: string; value: any }>,
    requestId?: string) => DataContentMessage;
  private readonly createStatusMessage: (code: number, statusMessage: string, requestId?: string) => StatusMessage;
  private readonly sendAlreadySubscribedErrorMsg: (ws: WebSocketWithId, vin: string, newDataPoints: string[]) => void;

  constructor(
    sendMessageToClient: (ws: WebSocketWithId, message: StatusMessage | DataContentMessage | ErrorMessage) => void,
    createDataContentMessage: (instance: string,
                               dataPoints: Array<{ name: string; value: any }>,
                               metadata: Array<{ name: string; value: any }>,
                               requestId?: string) => DataContentMessage,
    createStatusMessage: (code: number, statusMessage: string, requestId?: string) => StatusMessage,
    sendAlreadySubscribedErrorMsg: (ws: WebSocketWithId, vin: string, newDataPoints: string[]) => void) {
    this.session = new Session();
    this.notifyDatabaseChanges = this.notifyDatabaseChanges.bind(this);
    this.sendMessageToClient = sendMessageToClient;
    this.createDataContentMessage = createDataContentMessage;
    this.createStatusMessage = createStatusMessage;
    this.sendAlreadySubscribedErrorMsg = sendAlreadySubscribedErrorMsg;
    void this.session.authenticateAndConnect();

    // Register the cleanup function
    process.on("exit", this.cleanup.bind(this)); // Called when the process exits normally
    process.on("SIGINT", () => {
      this.cleanup();
      process.exit(0); // Ensure the process exits
    });
  }

  private async cleanup(): Promise<void> {
    logMessage("Cleaning up SubscriptionSimulator...");
    await this.session.closeSession(); // Replace with actual session close logic
  }

  /**
   * Creates a new subscription for the given websocket client for provided data points and VIN (message.instance)
   *
   * An associated client is represented by a websocket.
   * If the subscription exists, an error message is sent.
   * If a subscription was created and there is no timer yet, periodic database listener is enabled.
   * @param message subscribe message
   * @param wsOfNewSubscription websocket representing a client to be associated with the new subscription
   * @param newDataPoints list of data points the client wants to subscribe to
   * @returns void
   */
  async subscribe(message: SubscribeMessageType, wsOfNewSubscription: WebSocketWithId, newDataPoints: string[]): Promise<void> {
    const subscriptions: Subscription[] = this.websocketToSubscriptionsMap.get(wsOfNewSubscription) ?? [];
    let subscription = subscriptions.find(value => value.vin === message.instance);
    if (!subscription) {
      subscription = {vin: message.instance, dataPoints: new Set()};
      subscriptions.push(subscription); // Add to the array
    }

    const alreadySubscribed = newDataPoints.every(dataPoint => subscription.dataPoints.has(dataPoint));
    if (alreadySubscribed) {
      this.sendAlreadySubscribedErrorMsg(wsOfNewSubscription, message.instance, newDataPoints);
      return;
    }

    // Update subscribed data points
    newDataPoints.forEach(dataPoint => subscription.dataPoints.add(dataPoint));
    this.websocketToSubscriptionsMap.set(wsOfNewSubscription, subscriptions)

    this.startSubscriptionListenerIfNeeded();
    this.sendMessageToClient(wsOfNewSubscription, this.createStatusMessage(STATUS_SUCCESS.OK,
      `Successfully subscribed to '${message.instance}' [${toResponseFormat(newDataPoints)}].`
    ));

    logMessage("subscribed");
    this.logSubscriptions();
  }

  private startSubscriptionListenerIfNeeded() {
    if (this.intervalId === null) {
      this.intervalId = setInterval(this.notifyDatabaseChanges, databaseConfig!.pollIntervalLenInSec * 1000);
      this.intervalId.unref(); // Ensure timer doesn't keep the process alive
      this.timeIntervalLowerLimit = Date.now();
      logMessage("started timer");
    }
  }

  /**
   * Unsubscribes the client from the provided data points for given a given VIN (message.instance).
   * If a subscription with this exists for the given client, the client is removed from the subscription.
   * If no subscription exists, an error message is sent.
   * If no subscriptions are left for the client, it is removed.
   * If no clients are left, the periodic database listener is disabled.
   * @param message unsubscribe message
   * @param wsOfSubscriptionToBeDeleted websocket to be removed from subscription
   * @param dataPointsToUnsub list of data point strings to unsubscribe the client from
   * @returns void
   */
  unsubscribe(message: UnsubscribeMessageType, wsOfSubscriptionToBeDeleted: WebSocketWithId, dataPointsToUnsub: string[]): void {

    let subscription =
      this.websocketToSubscriptionsMap.get(wsOfSubscriptionToBeDeleted)?.find(value => value.vin === message.instance);

    if (!subscription) {
      this.sendMessageToClient(wsOfSubscriptionToBeDeleted, this.createStatusMessage(
        STATUS_ERRORS.NOT_FOUND,
        `Cannot unsubscribe. No subscription for instance '${message.instance}'`)
      );
      return;
    }

    const notSubscribed = dataPointsToUnsub.every(dataPoint => !subscription.dataPoints.has(dataPoint));

    if (notSubscribed) {
      this.sendMessageToClient(wsOfSubscriptionToBeDeleted, this.createStatusMessage(
        STATUS_ERRORS.NOT_FOUND,
        `Cannot unsubscribe. No subscription for instance '${message.instance}' ` +
        `and datapoints [${toResponseFormat(dataPointsToUnsub)}].`
      ));
      return;
    }

    // Remove the data points from the subscription
    dataPointsToUnsub.forEach((dataPoint) => subscription.dataPoints.delete(dataPoint));
    
    this.cleanUpEmptySubscriptions(subscription, wsOfSubscriptionToBeDeleted, message.instance);
    this.logSubscriptions();
  }

  /**
   * Cleans up client and/or it's subscriptions if they are not needed any more.
   * Stops the database listener if no clients are subscribed anymore.
   * @param subscription latest modified subscription
   * @param wsOfSubscriptionToBeDeleted websocket client
   * @param vin VIN the clients subscribed to
   * @private
   */
  private cleanUpEmptySubscriptions(subscription: any, wsOfSubscriptionToBeDeleted: WebSocketWithId, vin: string) {
    if (subscription.dataPoints.size == 0) {
      const subscriptions = this.websocketToSubscriptionsMap.get(wsOfSubscriptionToBeDeleted);
      if (subscriptions) {
        // Filter out the subscription to be deleted
        const updatedSubscriptions = subscriptions.filter(sub => sub.vin !== vin);

        if (updatedSubscriptions.length > 0) {
          // Update the Map with the new subscriptions array
          this.websocketToSubscriptionsMap.set(wsOfSubscriptionToBeDeleted, updatedSubscriptions);
        } else {
          // Remove the key from the Map if no subscriptions are left
          this.websocketToSubscriptionsMap.delete(wsOfSubscriptionToBeDeleted);
        }

        logMessage("unsubscribed");
        this.removeTimerIfNoSubscription();
        this.sendMessageToClient(
          wsOfSubscriptionToBeDeleted,
          this.createStatusMessage(STATUS_SUCCESS.OK, "Successfully unsubscribed"));
      }
    }
  }

  /**
   * Removes all subscriptions for a client.
   *
   * Checks all subscriptions and removes the given client from each of them.
   * If a subscription is left with no client, the entire subscription is removed.
   * If no subscription is left, the timer is stopped.
   * @param ws websocket representing a client to be removed from all subscriptions
   */
  unsubscribeClient(ws: WebSocketWithId): void {
    this.websocketToSubscriptionsMap.delete(ws);
    this.removeTimerIfNoSubscription();
    logMessage("unsubscribed client");
    this.logSubscriptions();
  }

  /**
   * Removes the timer if there is no subscription left.
   */
  private removeTimerIfNoSubscription() {
    if (this.intervalId !== null &&
      this.websocketToSubscriptionsMap.size === 0) {
      clearInterval(this.intervalId);
      this.intervalId = null;
      this.timeIntervalLowerLimit = undefined;
      logMessage("stopped timer");
    }
  }

  /**
   * Logs all subscriptions with their associated clients on the console.
   * Clients are represented by the ID of the websocket.
   */
  private logSubscriptions(): void {
    let logString = "subscriptions:\n";
    for (const [ws, subscriptions] of this.websocketToSubscriptionsMap.entries()) {
      logString += `WS id: ${ws.id} is subscribed to: \n`;
      subscriptions.forEach(sub =>
        logString += `  ${sub.vin} => [${Array.from(sub.dataPoints).join(", ")}] \n`
      )
    }
    logMessage(logString);
  }

  /**
   * Notifies subscribed clients about changes on data points during the last time interval.
   *
   * Checks for each subscription if there has been a change on the related data point in the last time interval.
   * For each change, it sends an update message for each client subscribed for this data point.
   *
   * @returns void
   */
  private async notifyDatabaseChanges(): Promise<void> {
    if (!this.timeIntervalLowerLimit) {
      logErrorStr("time of last check is undefined. Cannot go for notifications.");
      return;
    }

    const timeIntervalUpperLimit = Date.now();
    for (const [ws, subscriptions] of this.websocketToSubscriptionsMap.entries()) {
      await Promise.all(
        subscriptions.map(async (subscription) => {
          let dataContentMessage = await this.checkForChanges(
            subscription.vin, subscription.dataPoints, timeIntervalUpperLimit
          );
          if (dataContentMessage) {
            this.sendMessageToClient(ws, dataContentMessage);
          }
        })
      );
    }
    this.timeIntervalLowerLimit = timeIntervalUpperLimit;
  }

  /**
   * Checks for changes for the given vin in the last time interval.
   * For each change it creates an update message for each client subscribed for this data point
   * and returns the update message.
   *
   * @param vin VIN for checking value changes
   * @param dataPoints data points that are changes relevant
   * @param timeIntervalUpperLimit the upper limit of the time interval to be checked
   * @returns generated update message
   */
  private async checkForChanges(vin: string, dataPoints: Set<string>, timeIntervalUpperLimit: number): Promise<DataContentMessage | undefined> {
    const {databaseName, dataPointId} = databaseParams[TREE_VSS as keyof typeof databaseParams];
    const metadataPoints = [...dataPoints].map((dataPoint) => dataPoint + METADATA_SUFFIX)
    const fieldsToSearch = [...dataPoints, ...metadataPoints].join(", ");
    const sql = `SELECT ${fieldsToSearch}
                 FROM ${databaseName}
                 WHERE ${dataPointId} = '${vin}'
                   AND Time
                     > ${this.timeIntervalLowerLimit}
                   AND Time <= ${timeIntervalUpperLimit}
                 ORDER BY Time ASC`;

    let dataContentMessage = undefined;
    try {
      const sessionDataSet = await this.session.executeQueryStatement(sql);

      // Check if sessionDataSet is not an instance of SessionDataSet, and handle the error
      if (!(sessionDataSet instanceof SessionDataSet)) {
        throw new Error(
          "Failed to retrieve session data. Invalid session dataset."
        );
      }

      const [data, metadata] = transformSessionDataSet(sessionDataSet, databaseName);
      if (data.length > 0) {
        dataContentMessage = this.createDataContentMessage(
          vin, data, metadata
        );
        logMessage(`Processed ${data.length} changes for id ${vin}`);
      }

    } catch (error: unknown) {
      logError("Unknown error", error);
    } finally {
      return dataContentMessage;
    }
  }

}
