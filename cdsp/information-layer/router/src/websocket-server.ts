import { v4 as uuidv4 } from 'uuid';
import WebSocket, { RawData } from "ws";
import { RealmDBHandler } from "../../handlers/src/realmdb/src/RealmDbHandler";
import { IoTDBHandler } from "../../handlers/src/iotdb/src/IoTDBHandler";
import { HandlerBase } from "../../handlers/src/HandlerBase";
import { getHandlerType } from "../config/config";
import { validateMessage } from "../utils/message-validator";
import {
  logMessage,
  logError,
  logWithColor,
  MessageType,
  COLORS,
} from "../../utils/logger";
import { Message, WebSocketWithId } from "../../handlers/utils/data-types";

// Define the handler as the base class type
let handler: HandlerBase;

const handlerType: string = getHandlerType();

logWithColor(`\n ** Handler: ${handlerType} ** \n`, COLORS.BOLD);

// Instantiate the correct handler based on the handler type
switch (handlerType) {
  case "realmdb":
    handler = new RealmDBHandler();
    break;
  case "iotdb":
    handler = new IoTDBHandler();
    break;
  default:
    throw new Error("Unsupported handler type");
}

// WebSocket server creation
const server = new WebSocket.Server({ port: 8080 });

// Define clients Map globally to store connected clients
const clients = new Map<string, WebSocket>();

// Handle new client connections
server.on("connection", (ws: WebSocket) => {
  // add a uuid to the websocket connection
  const wsWithId = addIdToWebsocket(ws);
  const connectionId = wsWithId.id;
  clients.set(connectionId, ws); // Add client to the Map
  logWithColor(`Client connected with id ${connectionId}`, COLORS.YELLOW);

  // Handle messages from the client
  ws.on("message", (message: WebSocket.RawData) => {
    let messageString = rawDataToString(message);
    logMessage(JSON.stringify(messageString), MessageType.RECEIVED);
    const validatedMessage = validateMessage(messageString);

    if (validatedMessage instanceof Error) {
      logError("Invalid message format", validatedMessage);
      JSON.parse(validatedMessage.message).forEach((error: any) => {
        ws.send(JSON.stringify(error));
      });
    } else {
      // Pass the validated message to the handler
      handler.handleMessage(validatedMessage, connectionId, wsWithId);
    }
  });

  // Handle client disconnection
  ws.on("close", () => {
    handler.unsubscribe_client(ws); // Remove all client listeners
    clients.delete(connectionId);
    logWithColor(`Client disconnected with ID ${connectionId}`, COLORS.ORANGE);
  });
});

// Function to broadcast messages to all connected clients
const sendMessageToClients = (message: Message) => {
  logMessage(JSON.stringify(message), MessageType.SENT);
  clients.forEach((client) => {
    client.send(JSON.stringify(message));
  });
};

// Initialize handler and authenticate
logMessage(`Starting authentication and connection ...`);
handler.authenticateAndConnect(sendMessageToClients);

// Log server start
logWithColor(`Web-Socket server started on ws://localhost:8080\n`, COLORS.BOLD);

function addIdToWebsocket(ws: WebSocket): WebSocketWithId {
  return Object.assign(ws, { id: uuidv4() });
}


function rawDataToString(message: RawData): string {
  if (typeof message === "string") {
    return message;
  } else if (Buffer.isBuffer(message)) {
    return message.toString();
  } else if (message instanceof ArrayBuffer) {
    return new TextDecoder().decode(message);
  } else {
    throw new Error("Unsupported message type");
  }
}
