import {v4 as uuidv4} from 'uuid';
import WebSocket, {RawData} from "ws";
import {createHandler} from "../../handlers/src/HandlerCreator";
import {RequestValidator} from '../utils/RequestValidator';
import {
  logMessage,
  logWithColor,
  LogMessageType,
  COLORS,
  logErrorStr,
} from "../../utils/logger";
import {WebSocketWithId} from "../../utils/database-params";
import {ErrorMessage, STATUS_ERRORS} from "../utils/ErrorMessage";
// WebSocket server creation
const server = new WebSocket.Server({port: 8080});

// Define clients Map globally to store connected clients
const clients = new Map<string, WebSocket>();

// Handle new client connections
server.on("connection", (ws: WebSocket) => {
  // add an uuid to the websocket connection
  const wsWithId = addIdToWebsocket(ws);
  const connectionId = wsWithId.id;
  clients.set(connectionId, ws); // Add client to the Map
  const handler = createHandler();
  handler.authenticateAndConnect();
  logWithColor(`Client connected and authenticated with id ${connectionId}`, COLORS.YELLOW);

  // Handle messages from the client
  ws.on("message", (message: WebSocket.RawData) => {
    let messageString = rawDataToString(message);
    logMessage(JSON.stringify(messageString), LogMessageType.RECEIVED, connectionId);

    // validation
    const validator = new RequestValidator();
    const result = validator.validate(messageString); // new validation

    if (result.valid) {
      // Handle valid Message and exit
      handler.handleMessage(result.message!, wsWithId);
      return;
    }

    // Unified error section
    logErrorStr("Processing failed:");
    result.errors?.forEach((error) => {
      logErrorStr(error);
      const statusMessage = {
        type: "status",
        code: STATUS_ERRORS.BAD_REQUEST,
        message: `Processing Error: ${error}`,
      }
      ws.send(JSON.stringify(statusMessage));
    });

  });

  // Handle client disconnection
  ws.on("close", () => {
    handler.unsubscribe_client(wsWithId); // Remove all client listeners
    clients.delete(connectionId);
    logWithColor(`Client disconnected with ID ${connectionId}`, COLORS.ORANGE);
  });
});

// Log server start
logWithColor(`Web-Socket server started on ws://localhost:8080\n`, COLORS.BOLD);

function addIdToWebsocket(ws: WebSocket): WebSocketWithId {
  return Object.assign(ws, {id: uuidv4()});
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
