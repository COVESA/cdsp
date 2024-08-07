const WebSocket = require("ws");
const RealmDBHandler = require("../../handlers/realmdb/src/realmdb-handler");
const IoTDBHandler = require("../../handlers/iotdb/src/iotdb-handler");
const { getHandlerType } = require("../config/config");
const { validateMessage } = require("../utils/message-validator");

const handlerType = getHandlerType();
let handler;
console.log(`this is the handler type: ${handlerType}`);

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
  console.log("Client connected");
  clients.push(ws); // Add client to the array

  ws.on("message", (message) => {
    console.log(`Message received: ${message}`);
    const validatedMessage = validateMessage(message);
    if (validatedMessage instanceof Error) {
      console.error(`Invalid message format: ${validatedMessage.message}`);
      ws.send(
        JSON.stringify({
          error: `Invalid message format`,
        })
      );
    } else {
      handler.handleMessage(validatedMessage, ws);
    }
  });

  ws.on("close", () => {
    console.log("Client disconnected");
    // Remove disconnected client from the array
    clients = clients.filter((client) => client !== ws);
  });
});

const sendMessageToClients = (message) => {
  clients.forEach((client) => {
    client.send(JSON.stringify(message));
  });
};

console.log(`Starting authentication and connection to ${handlerType} ...`);
handler.authenticateAndConnect(sendMessageToClients);

console.log(
  `WebSocket server started on ws://localhost:8080 using ${handlerType} handler`
);