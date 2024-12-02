import { logMessage, logError, logErrorStr } from "../../../../utils/logger";
import { databaseConfig, databaseParams } from "../config/database-params";
import { Session } from "./Session";
import { SessionDataSet } from "../utils/SessionDataSet";
import { WebSocketWithId, Message, MessageBase, ErrorMessage, STATUS_ERRORS} from "../../../utils/data-types";
import { createErrorMessage } from "../../../../utils/error-message-helper";
import { transformSessionDataSet } from "../utils/database-helper";

type SubscriptionMap = Map<string, WebSocketWithId[]>;

const RANDOM_STRING = "<random-string>";
const TREE_VSS = "VSS";

// Define the singleton instance at the module level
let subscriptionSimulatorInstance: SubscriptionSimulator | null = null;

// Function to get or initialize the singleton instance
export function getSubscriptionSimulator(
  sendMessageToClient: (ws: WebSocketWithId, message: Message | MessageBase | ErrorMessage) => void,
  createUpdateMessage: (id: string, tree: string, uuid: string, nodes: Array<{ name: string; value: any }>) => Message,
  createSubscribeStatusMessage: (type: "subscribe" | "unsubscribe", message: Pick<Message, "id" | "tree" | "uuid">, status: string) => MessageBase
): SubscriptionSimulator {
  if (!subscriptionSimulatorInstance) {
    subscriptionSimulatorInstance = new SubscriptionSimulator(sendMessageToClient, createUpdateMessage, createSubscribeStatusMessage);
    logMessage("SubscriptionSimulator instance created.");
  }
  return subscriptionSimulatorInstance;
}

export class SubscriptionSimulator {
  private intervalId: NodeJS.Timeout | null = null;
  private session: Session;
  private timeIntervalLowerLimit: number | undefined = undefined;
  private subscriptions: SubscriptionMap = new Map();
  private sendMessageToClient: ( ws: WebSocketWithId, message: Message | MessageBase | ErrorMessage) => void;  
  private createUpdateMessage: (id: string, tree: string, uuid: string, nodes: Array<{ name: string; value: any }>) => Message;
  private createSubscribeStatusMessage: (type: "subscribe" | "unsubscribe", message: Pick<Message, "id" | "tree" | "uuid">, status: string) => MessageBase;
  
  constructor(
    sendMessageToClient: (ws: WebSocketWithId, message: Message | MessageBase | ErrorMessage) => void,
    createUpdateMessage: (id: string, tree: string, uuid: string, nodes: Array<{ name: string; value: any }>) => Message,
    createSubscribeStatusMessage: (type: "subscribe" | "unsubscribe", message: Pick<Message, "id" | "tree" | "uuid">, status: string) => MessageBase)
  {
    this.session = new Session();
    this.notifyDatabaseChanges = this.notifyDatabaseChanges.bind(this);    
    this.sendMessageToClient = sendMessageToClient;
    this.createUpdateMessage = createUpdateMessage;
    this.createSubscribeStatusMessage = createSubscribeStatusMessage;
    this.session.authenticateAndConnect();

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
   * Creates a new subscription for the given websocket.
   * 
   * A subscription is identified by an id/tree combination key.
   * An associated client is represented by a websocket.
   * If a subscription with this key does not exist, it is created with the client.
   * If such a subscription already exists, but not for the given client, the client is added to the sunscription.
   * If the subscription exists with this client, an error message is sent.
   * If a subscription was created and there is no timer yet, a timer is started.
   * @param message subscribe message
   * @param wsOfNewSubscription websocket representing a client to be associated with the new subscription
   * @returns void
   */
  async subscribe(message: Message, wsOfNewSubscription: WebSocketWithId): Promise<void> {
    const key = `${message.id}-${message.tree}`;
    const websockets = this.subscriptions.get(key);

    if (websockets && websockets.some(ws => ws.id === wsOfNewSubscription.id)) {
      this.sendMessageToClient(wsOfNewSubscription,
        createErrorMessage(
          "subscribe",
          STATUS_ERRORS.BAD_REQUEST,
          `Subscription already done to id ${message.id} and tree ${message.tree} for connection ${wsOfNewSubscription.id}`));
          return;
    }
    if (websockets) {
      websockets.push(wsOfNewSubscription);
    } else {
      this.subscriptions.set(key, [wsOfNewSubscription]);
    }

    if (this.intervalId === null) {
      this.intervalId = setInterval(this.notifyDatabaseChanges, databaseConfig!.pollIntervalLenInSec * 1000);
      this.intervalId.unref(); // Ensure timer doesn't keep the process alive
      this.timeIntervalLowerLimit = Date.now();

      logMessage("started timer");
    }

    this.sendMessageToClient(
      wsOfNewSubscription,
      this.createSubscribeStatusMessage("subscribe", message, "succeed")
    );

    logMessage("subscribed");
    this.logSubscriptions();
  }

  /**
   * Removes a subscription for the given websocket.
   * 
   * A subscription is identified by an id/tree combination key.
   * An associated client is represented by a websocket.
   * If a subscription with this key exists for the given client, the client is removed from the subscription.
   * If there is no other client left for this subscription, the entire subscription is removed.
   * If subscription does not exist, an error message is sent.
   * If no subscription is left, the timer is stopped.
   * @param message unsubscribe message
   * @param wsOfSubscriptionToBeDeleted websocket to be removed from subscription
   * @returns void
   */
  unsubscribe(message: Message, wsOfSubscriptionToBeDeleted: WebSocketWithId): void {
    const key = `${message.id}-${message.tree}`;
    const websockets = this.subscriptions.get(key);

    if (!websockets) {
      this.sendMessageToClient(wsOfSubscriptionToBeDeleted,
        createErrorMessage(
          "unsubscribe",
          STATUS_ERRORS.BAD_REQUEST,
          `Cannot unsubscribe. No subscription for id ${message.id}, tree ${message.tree} for connection ${wsOfSubscriptionToBeDeleted.id}.`));
          return;
    }
    const index = websockets.indexOf(wsOfSubscriptionToBeDeleted);
    if (index > -1) {
      websockets.splice(index, 1);
      if (websockets.length === 0) {
        this.subscriptions.delete(key);
      }
    } 

    this.removeTimerIfNoSubscription();

    this.sendMessageToClient(
      wsOfSubscriptionToBeDeleted,
      this.createSubscribeStatusMessage("unsubscribe", message, "succeed")
    );

    logMessage("unsubscribed");
    this.logSubscriptions();
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
    this.removeWebSocketFromSubscriptions(ws);
    this.removeTimerIfNoSubscription();
    logMessage("unsubscribed client");
    this.logSubscriptions();
  }

  /**
   * Removes the timer if there is no subscription left.
   */
  private async removeTimerIfNoSubscription() {
    if (this.intervalId !== null &&
      this.subscriptions.size === 0) {
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
  private logSubscriptions() : void {
    let logString = "subscriptions:\n";
    for (const [key, websockets] of this.subscriptions.entries()) {
      logString += `${key} ws ids: \n`;
      websockets.forEach(ws =>
        logString += `  ${ws.id} \n`
      )
    };
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
  private async notifyDatabaseChanges() : Promise<void> {
    if (!this.timeIntervalLowerLimit) {
        logErrorStr("time of last check is undefined. Cannot go for notifications.");
        return;
    }
    
    const timeIntervalUpperLimit = Date.now();
    for (const [key, websockets] of this.subscriptions.entries()) {
      const [id, tree] = key.split('-');

      const updateMessage = await this.checkForChanges(id, timeIntervalUpperLimit);
      if (updateMessage) {
        websockets.forEach(ws =>
          this.sendMessageToClient(ws, updateMessage)
        );
      }
    }
    this.timeIntervalLowerLimit = timeIntervalUpperLimit;
  }

  /**
   * Checks for changes for the given id in the last time interval.
   * For each change it creates an update message for each client subscribed for this data point
   * and returns the update message.
   *
   * @param id ID to be checked
   * @param timeIntervalUpperLimit the upper limit of the time interval to be checked
   * @returns generated update message
   */
  private async checkForChanges(id: string, timeIntervalUpperLimit: number): Promise<Message | undefined> {
    const { databaseName, dataPointId } =
    databaseParams[TREE_VSS as keyof typeof databaseParams];

    const sql = `SELECT * FROM ${databaseName} 
                          WHERE ${dataPointId} = '${id}'
                          AND Time > ${this.timeIntervalLowerLimit} AND Time <= ${timeIntervalUpperLimit}
                          ORDER BY Time ASC`;
    let updateMessage = undefined;
    try {
      const sessionDataSet = await this.session.executeQueryStatement(sql);

      // Check if sessionDataSet is not an instance of SessionDataSet, and handle the error
      if (!(sessionDataSet instanceof SessionDataSet)) {
        throw new Error(
          "Failed to retrieve session data. Invalid session dataset."
        );
      }

      const responseNodes = transformSessionDataSet(sessionDataSet, databaseName);
      if (responseNodes.length > 0) {
        updateMessage =  this.createUpdateMessage(
          id, "VSS", RANDOM_STRING,
          responseNodes
        );
      }
      if (responseNodes.length > 0) {
        logMessage(`Processed ${responseNodes.length} changes for id ${id}`);
      }

    } catch (error: unknown) {
      logError("Unknown error", error);
    } finally {
      return updateMessage;
    }
  }

  /**
   * Removes client from all subscriptions.
   * 
   * Checks all subscriptions and removes the given client from each of them.
   * If a subscription is left with no client, the entire subscription is removed.
   * @param webSocketToBeRemoved websocket representing the client to be removed
   */
  private removeWebSocketFromSubscriptions(
    webSocketToBeRemoved: WebSocketWithId
  ): void {
    for (const [key, websockets] of this.subscriptions.entries()) {
      // Find the index of the target WebSocket in the websockets array
      const wsIndex = websockets.findIndex(ws => ws.id === webSocketToBeRemoved.id);
  
      // If found, remove the WebSocket from the array
      if (wsIndex !== -1) {
        websockets.splice(wsIndex, 1);  // Remove the WebSocket at wsIndex
  
        // If websockets array is now empty, delete the subscription
        if (websockets.length === 0) {
          this.subscriptions.delete(key);
        }
      }
    }
  }  
}
