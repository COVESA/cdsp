const WebSocket = require("ws");
const {
  authenticateAndConnectToRealm,
  onMediaElementChange,
} = require("../../realmdb/src/realm-handler");

const server = new WebSocket.Server({ port: 8080 });

// Define clients array globally
let clients = [];

server.on("connection", (ws) => {
  console.log("Client connected");
  clients.push(ws); // Add client to the array

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

console.log("Starting authentication and connection to Realm...");
authenticateAndConnectToRealm(sendMessageToClients, onMediaElementChange);
