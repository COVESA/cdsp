import {v4 as uuidv4} from 'uuid';
import WebSocket, {RawData} from "ws";
import {createHandler} from "../../handlers/src/HandlerCreator";
import {RequestValidator} from '../utils/RequestValidator';
import {COLORS, logErrorStr, logMessage, LogMessageType, logWithColor,} from "../../utils/logger";
import {WebSocketWithId} from "../../utils/database-params";
import {HandlerBase} from "../../handlers/src/HandlerBase";
import {MessageMapper} from "../utils/MessageMapper";
import {DataContentMessage, ErrorMessage, STATUS_ERRORS, StatusMessage} from "../utils/NewMessage";
// WebSocket server creation
const server = new WebSocket.Server({port: 8080});

// Define clients Map globally to store connected clients
const clients = new Map<string, WebSocket>();
const messageMapper = new MessageMapper();

// Handle new client connections
server.on("connection", (ws: WebSocket) => {
  // Temporarily pause the WebSocket connection to prevent processing incoming messages 
  // until authentication is successfully completed. Once authenticated, resume the connection.
  ws.pause()

  // add an uuid to the websocket connection
  const wsWithId = addIdToWebsocket(ws);
  const connectionId = wsWithId.id;
  clients.set(connectionId, ws); // Add client to the Map
  const handler = createHandler(sendMessage);

  // Handle messages from the client
  ws.on("message", handleMessage(connectionId, handler, wsWithId));

  // Handle client disconnection
  ws.on("close", handleCloseConnection(handler, wsWithId, connectionId));

  handler.authenticateAndConnect().then(_ => ws.resume()); // continue message processing
  logWithColor(`Client connected and authenticated with id ${connectionId}`, COLORS.YELLOW);
});

function handleMessage(connectionId: string, handler: HandlerBase, wsWithId: WebSocketWithId) {
  return (message: WebSocket.RawData) => {
    let messageString = rawDataToString(message);
    logMessage(messageString, LogMessageType.RECEIVED, connectionId);

    // validation
    const validator = new RequestValidator();
    const result = validator.validate(messageString);

    if (result.valid) {
      // Map DTO to internal business object
      const messageBO = messageMapper.toDomain(result.messageDTO!);
      // Handle valid Message and exit
      handler.handleMessage(messageBO, wsWithId);
      return;
    }

    // Unified error section
    logErrorStr("Processing failed:");
    result.errors?.forEach((error) => {
      logErrorStr(error);
      const errorMessage: ErrorMessage = {
        code: STATUS_ERRORS.BAD_REQUEST,
        type: "error",
        message: `Processing Error: ${error}`,
        reason:`Processing Error: ${error}`,
        requestId: result.messageDTO?.id
      }
      sendMessage(wsWithId, errorMessage);
    });
  };
}

function handleCloseConnection(handler: HandlerBase, wsWithId: WebSocketWithId, connectionId: string) {
  return () => {
    handler.unsubscribe_client(wsWithId); // Remove all client listeners
    clients.delete(connectionId);
    logWithColor(`Client disconnected with ID ${connectionId}`, COLORS.ORANGE);
  };
}

// Log server start
logWithColor(`Web-Socket server started on ws://localhost:8080\n`, COLORS.BOLD);

function addIdToWebsocket(ws: WebSocket): WebSocketWithId {
  return Object.assign(ws, {id: uuidv4()});
}

/**
 * This must be the only function that sends messages via websocket to the connected clients to guarantee the right message structure. 
 * Don't use WebSocket.send(...) anywhere else.
 * @param ws websocket object
 * @param message message to send
 */
export function sendMessage(
  ws: WebSocketWithId,
  message: StatusMessage | DataContentMessage | ErrorMessage
): void {
  
  let dataResponseDTO = messageMapper.toDTO(message);
  ws.send(JSON.stringify(dataResponseDTO));

  logMessage(JSON.stringify(dataResponseDTO, null, 2), LogMessageType.SENT, ws.id);
}

function rawDataToString(message: RawData): string {
  if (Buffer.isBuffer(message)) {
    return message.toString();
  } else if (message instanceof ArrayBuffer) {
    return new TextDecoder().decode(message);
  } else {
    throw new Error("Unsupported message type");
  }
}
