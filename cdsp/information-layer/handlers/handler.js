class Handler {
  authenticateAndConnect(sendMessageToClients) {
    throw new Error("Method 'authenticateAndConnect' must be implemented.");
  }

  handleMessage(message, ws) {
    switch (message.type) {
      case "read":
        this.read(message, ws);
        break;
      case "write":
        this.write(message, ws);
        break;
      case "subscribe":
        this.subscribe(message, ws);
        break;
      default:
        ws.send(JSON.stringify({ error: "Unknown message type" }));
    }
  }

  read(message, ws) {
    throw new Error("Read method not implemented, yet!");
  }

  write(message, ws) {
    throw new Error("Write method not implemented, yet!");
  }

  subscribe(message, ws) {
    throw new Error("Subscribe method not implemented, yet!");
  }
}

module.exports = Handler;
