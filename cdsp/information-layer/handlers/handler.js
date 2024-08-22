class Handler {
  authenticateAndConnect(sendMessageToClients) {
    throw new Error("Method 'authenticateAndConnect' must be implemented.");
  }

  handleMessage(message, ws) {
    try {
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
    } catch (error) {
      ws.send(JSON.stringify({ error: error.message }));
    }
  }

  read(message, ws) {
    throw new Error("Read method not implemented yet!");
  }

  write(message, ws) {
    throw new Error("Write method not implemented yet!");
  }

  subscribe(message, ws) {
    throw new Error("Subscribe method not implemented yet!");
  }

  /**
   * Utility functions for handling messages and data structures common to IoTDB and RealmDB.
   */

  /**
   * Sends a message to the client
   * @param {WebSocket} ws - The WebSocket connection to send the response to.
   * @param {Object} message - The message to be sent to the client.
   */
  sendMessageToClient = (ws, message) => {
    ws.send(JSON.stringify(message));
  };

  /**
   * Generic function to create or update a message.
   * @param {Object} message - The original message.
   * @param {Array} nodes - The nodes to be included in the message.
   * @param {String} type - The type of the message. Should be either 'read' or 'update'.
   * @returns {Object} - The transformed message.
   */
  createOrUpdateMessage(message, nodes, type) {
    const { id, tree, uuid } = message;
    let newMessage = {
      type,
      tree,
      id,
      dateTime: new Date().toISOString(),
      uuid,
    };
    if (nodes.length === 1) {
      newMessage["node"] = nodes[0];
    } else {
      newMessage["nodes"] = nodes;
    }
    return newMessage;
  }

  /**
   * Transforms a message node by replacing all dots with underscores.
   *
   * @param {string} node - The message node to transform.
   * @returns {string} - The transformed message node with dots replaced by underscores.
   */
  transformEndpointFromMessageNode(node) {
    return `${node}`.replace(/\./g, "_");
  }

  /**
   * Transforms a database field name by replacing underscores with all dots.
   *
   * @param {string} field - The database filed to transform.
   * @returns {string} - The transformed to message node replacing underscores by dots.
   */
  transformEndpointFromDatabaseFields(field) {
    return `${field}`.replace(/\_/g, ".");
  }
}

module.exports = Handler;
