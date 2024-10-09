const WebSocket = require("ws");
const RealmDBHandler = require("../../handlers/src/realmdb/src/realmdb-handler");
const IoTDBHandler = require("../../handlers/src/iotdb/src/iotdb-handler");
const { getHandlerType } = require("../config/config");
const { validateMessage } = require("../utils/message-validator");
const {
  logMessage,
  logWithColor,
  MessageType,
  COLORS,
} = require("../../utils/logger");

const handlerType = getHandlerType();
let handler;

logWithColor(`\n ** Handler: ${handlerType} ** \n`, COLORS.BOLD);

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

const server = new WebSocket.Server({ port: 8080 });

// Define clients array globally
let clients = [];

server.on("connection", (ws) => {
  logWithColor("* Client connected *", COLORS.YELLOW);
  clients.push(ws); // Add client to the array

  ws.on("message", (message) => {
    logMessage(JSON.stringify(message), MessageType.RECEIVED);
    const validatedMessage = validateMessage(message);

    if (validatedMessage instanceof Error) {
      logMessage(
        `Invalid message format: ${validatedMessage.message}`,
        MessageType.ERROR
      );

      JSON.parse(validatedMessage.message).forEach((error) => {
        ws.send(JSON.stringify(error));
      });
    } else {
      handler.handleMessage(validatedMessage, ws);
    }
  });

  ws.on("close", () => {
    // Remove all client listeners
    handler.unsubscribe_client(ws);
    // Remove disconnected client from the array
    clients = clients.filter((client) => client !== ws);
    console.info("* Client disconnected *");
  });
});

const sendMessageToClients = (message) => {
  logMessage(JSON.stringify(message), MessageType.SENT);
  clients.forEach((client) => {
    client.send(JSON.stringify(message));
  });
};

console.info(`Starting authentication and connection ...`);
handler.authenticateAndConnect(sendMessageToClients);
logWithColor(`Web-Socket server started on ws://localhost:8080\n`, COLORS.BOLD);
