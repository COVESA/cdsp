const Client = require("../gen-nodejs/IClientRPCService");
const {
  TSExecuteStatementReq,
  TSOpenSessionReq,
  TSProtocolVersion,
  TSCloseSessionReq,
  TSInsertRecordReq,
} = require("../gen-nodejs/client_types");
const thrift = require("thrift");
const Handler = require("../../handler");
const config = require("../config/config");
const SessionDataSet = require("../utils/SessionDataSet");
const { IoTDBDataInterpreter } = require("../utils/IoTDBDataInterpreter");
const MeasurementType = require("../config/MeasurementsType");
const Database = require("../config/Database");

const sendMessageToClient = (ws, message) => {
  ws.send(JSON.stringify(message));
};

/**
 * Extracts endpoint names from the given message.
 *
 * This function checks if the message has a single node or multiple nodes and
 * extracts the names accordingly.
 *
 * @param {Object} message - The message containing node(s).
 * @returns {Array<string>} An array of endpoint names.
 */
function extractEndpointsFromNodes(message) {
  let endpoints = [];
  if (message.node) {
    endpoints.push(message.node.name);
  } else if (message.nodes) {
    endpoints = message.nodes.map((node) => node.name);
  }
  return endpoints;
}

/**
 * Creates an object to insert data in IoTDB based on the provided message.
 *
 * @param {Object} message - The message containing data to be inserted.
 * @returns {Object} The data object to be inserted.
 */
function createObjectToInsert(message) {
  const data = { VehicleIdentification_VIN: message.id };
  if (message.node) {
    data[message.node.name] = message.node.value;
  } else if (message.nodes) {
    message.nodes.forEach((node) => {
      data[node.name] = node.value;
    });
  }
  return data;
}

/**
 * Creates an update message object based on the provided message and response nodes.
 *
 * @param {Object} message - The original message object containing tree, id, and uuid properties.
 * @param {Array} responseNodes - An array of response nodes to be included in the update message.
 * @returns {Object} The constructed update message object.
 */
function createUpdateMessage(message, responseNodes) {
  let updateMessage = {
    type: "update",
    tree: message.tree,
    id: message.id,
    dateTime: new Date().toISOString(),
    uuid: message.uuid,
  };
  if (responseNodes.length == 1) {
    updateMessage["node"] = responseNodes[0];
  } else {
    updateMessage["nodes"] = responseNodes;
  }
  return updateMessage;
}

/**
 * Creates a read message object based on the provided message and nodes names.
 *
 * @param {Object} message - The message object containing tree, id, and uuid properties.
 * @param {Array} nodeNames - An array of node names. If the array contains one field, it will be added as 'node'.
 *                          If the array contains multiple fields, they will be added as 'nodes'.
 * @returns {Object} The constructed read message object.
 */
function createReadMessage(message, nodeNames) {
  let readMessage = {
    type: "read",
    tree: message.tree,
    id: message.id,
    uuid: message.uuid,
  };
  if (nodeNames.length == 1) {
    readMessage["node"] = nodeNames[0];
  } else {
    readMessage["nodes"] = nodeNames;
  }
  return readMessage;
}

class IoTDBHandler extends Handler {
  constructor() {
    super();
    this.client = null;
    this.sendMessageToClients = null;
    this.sessionId = null;
    this.protocolVersion = TSProtocolVersion.IOTDB_SERVICE_PROTOCOL_V3;
    this.statementId = null;
    this.zoneId = config.timeZoneId;
    this.fetchSize = config.fetchSize;
    this.isSessionClosed = true;
  }

  async authenticateAndConnect(sendMessageToClients) {
    try {
      this.sendMessageToClients = sendMessageToClients;

      const connection = thrift.createConnection(
        config.iotdbHost,
        config.iotdbPort,
        {
          transport: thrift.TFramedTransport,
          protocol: thrift.TBinaryProtocol,
        }
      );

      this.client = thrift.createClient(Client, connection);

      connection.on("error", (err) => {
        console.error("thrift connection error:", err);
      });

      console.log("Successfully connected to IoTDB using thrift.");
    } catch (error) {
      console.error("Failed to authenticate with IoTDB:", error);
    }
  }

  async read(message, ws) {
    try {
      await this.#openSessionIfNeeded();
      const responseMessage = await this.#queryLastFields(message, ws);
      console.log(responseMessage);
      sendMessageToClient(ws, JSON.stringify(responseMessage));
    } catch (error) {
      console.error("Failed to read data from IoTDB: ", error);
      sendMessageToClient(
        ws,
        JSON.stringify({ error: "Error reading object" })
      );
    } finally {
      this.#closeSessionIfNeeded();
    }
  }

  async write(message, ws) {
    let measurements = [];
    let dataTypes = [];
    let values = [];

    try {
      await this.#openSessionIfNeeded();
      const data = createObjectToInsert(message);
      const errorUndefinedTypes = [];

      for (const [key, value] of Object.entries(data)) {
        const dataType = MeasurementType[key];
        if (dataType == undefined) {
          errorUndefinedTypes.push(`The endpoint "${key}" is not supported`);
          continue;
        }
        measurements.push(key);
        dataTypes.push(MeasurementType[key]);
        values.push(value);
      }

      if (errorUndefinedTypes.length > 0) {
        errorUndefinedTypes.forEach((error) => console.error(error));
        throw new Error("One or more endpoints are not supported.");
      }

      const timestamp = new Date().getTime();
      const deviceId = Database[message.tree].database_name;
      const status = await this.#insertRecord(
        deviceId,
        timestamp,
        measurements,
        dataTypes,
        values
      );

      const response = `insert one record to device ${deviceId}, status: `;
      console.log(response, status);

      let keyList = [];
      measurements.forEach((key) => {
        keyList.push({ name: key });
      });

      const readMessage = createReadMessage(message, keyList);
      console.log(readMessage);

      const responseMessage = await this.#queryLastFields(message, ws);
      console.log(responseMessage);
      this.sendMessageToClients(responseMessage);
    } catch (error) {
      console.error("Failed to write data from IoTDB: ", error);
      sendMessageToClient(
        ws,
        JSON.stringify({ error: "Error writing object" })
      );
    } finally {
      this.#closeSessionIfNeeded();
    }
  }

  /**
   * Opens a session with the IoTDB server using the provided credentials and configuration.
   */
  async #openSession() {
    if (!this.isSessionClosed) {
      console.info("The session is already opened.");
      return;
    }

    const openSessionReq = new TSOpenSessionReq({
      username: config.iotdbUser,
      password: config.iotdbPassword,
      client_protocol: this.protocolVersion,
      zoneId: config.timeZoneId,
      configuration: { version: "V_0_13" },
    });

    try {
      const resp = await this.client.openSession(openSessionReq);

      if (this.protocolVersion != resp.serverProtocolVersion) {
        console.log(
          "Protocol differ, Client version is " +
            this.protocolVersion +
            ", but Server version is " +
            resp.serverProtocolVersion
        );
        // version is less than 0.10
        if (resp.serverProtocolVersion == 0) {
          throw new Error("Protocol not supported.");
        }
      }

      this.sessionId = resp.sessionId;
      this.statementId = await this.client.requestStatementId(this.sessionId);
      this.isSessionClosed = false;
      console.log("Session started!");
    } catch (error) {
      console.error("Failed starting session with IoTDB: ", error);
    }
  }

  /**
   * Closes the current session if it is not already closed.
   */
  #closeSession() {
    if (this.isSessionClosed) {
      console.info("Session is already closed.");
      return;
    }

    let req = new TSCloseSessionReq({
      sessionId: this.sessionId,
    });

    try {
      this.client.closeSession(req);
    } catch (error) {
      console.error(
        "Error occurs when closing session at server. Maybe server is down. Error message: ",
        error
      );
    } finally {
      this.isSessionClosed = true;
      console.log("Session closed!");
    }
  }

  async #openSessionIfNeeded() {
    if (this.isSessionClosed) {
      await this.#openSession();
    }
  }

  async #closeSessionIfNeeded() {
    if (!this.isSessionClosed) {
      this.#closeSession();
    }
  }

  /**
   * Executes a SQL query statement asynchronously.
   *
   * @param {string} sql - The SQL query statement to be executed.
   * @returns {Promise<SessionDataSet|Object>} - Returns a SessionDataSet object if the query is successful,
   *                                             otherwise returns an empty object.
   * @throws {Error} - Throws an error if the session is not open.
   */
  async #executeQueryStatement(sql) {
    try {
      if (!this.sessionId) {
        throw new Error("Session is not open. Please authenticate first.");
      }

      const request = new TSExecuteStatementReq({
        sessionId: this.sessionId,
        statement: sql,
        statementId: this.statementId,
        fetchSize: this.fetchSize,
        timeout: 0,
      });

      const resp = await this.client.executeQueryStatement(request);

      if (!resp || !resp.queryDataSet || !resp.queryDataSet.valueList) {
        return {};
      } else {
        return new SessionDataSet(
          resp.columns,
          resp.dataTypeList,
          resp.columnNameIndexMap,
          resp.queryId,
          this.client,
          this.statementId,
          this.sessionId,
          resp.queryDataSet,
          resp.ignoreTimeStamp
        );
      }
    } catch (error) {
      console.error("Failed executing query statement: ", error);
    }
  }

  /**
   * Inserts a record into the time series database.
   * @param {string} deviceId - The ID of the device.
   * @param {number} timestamp - The timestamp of the record.
   * @param {string[]} measurements - Array of measurement names.
   * @param {string[]} dataTypes - Array of data types for each value.
   * @param {any[]} values - Array of values to be inserted.
   * @param {boolean} isAligned - Flag indicating if the data is aligned.
   * @returns {Promise<any>} - A promise that resolves with the result of the insertion.
   * @throws {string} - Throws an error if lengths of data types, values, and measurements do not match.
   */
  async #insertRecord(
    deviceId,
    timestamp,
    measurements,
    dataTypes,
    values,
    isAligned = false
  ) {
    if (
      values.length != dataTypes.length ||
      values.length != measurements.length
    ) {
      throw "length of data types does not equal to length of values!";
    }
    const valuesInBytes = IoTDBDataInterpreter.serializeValues(
      dataTypes,
      values
    );

    let request = new TSInsertRecordReq({
      sessionId: this.sessionId,
      prefixPath: deviceId,
      measurements: measurements,
      values: valuesInBytes,
      timestamp: timestamp,
      isAligned: isAligned,
    });
    return await this.client.insertRecord(request);
  }

  /**
   * Queries the last inserted fields from the database based on the provided message and sends the result back.
   *
   * @param {Object} message - The message object containing the query parameters.
   * @param {WebSocket} ws - The WebSocket connection to send the response back to the client.
   * @returns {Promise<void>} - A promise that resolves when the query is complete and the response is sent.
   * @throws {Error} - Throws an error if the query execution fails.
   */
  async #queryLastFields(message, ws) {
    const objectId = message.id;
    const databaseName = Database[message.tree].database_name;
    const endpointId = Database[message.tree].endpoint_id;
    const fieldsToSearch = extractEndpointsFromNodes(message).join(", ");
    const sql = `SELECT ${fieldsToSearch} FROM ${databaseName} WHERE ${endpointId} = '${objectId}' ORDER BY Time DESC LIMIT 1`;

    try {
      const sessionDataSet = await this.#executeQueryStatement(sql);

      // Check if the response contains data
      if (sessionDataSet == {}) {
        console.log("No data found in query response.");
        sendMessageToClient(
          ws,
          JSON.stringify({ error: "No data found in query response." })
        );
      } else {
        let mediaElements = [];
        while (sessionDataSet.hasNext()) {
          const mediaElement = sessionDataSet.next();
          mediaElements.push(mediaElement);
        }

        let responseNodes = [];

        if (mediaElements.length > 0) {
          for (let i = 0; i < mediaElements.length; ++i) {
            const transformedObject =
              IoTDBDataInterpreter.extractDeviceIdFromTimeseries(
                mediaElements[i]
              );
            const entries = Object.entries(transformedObject);
            entries.forEach(([key, value]) => {
              responseNodes.push({ name: key, value: value });
            });
          }
          return createUpdateMessage(message, responseNodes);
        } else {
          sendMessageToClient(
            ws,
            JSON.stringify({ error: "Object not found." })
          );
        }
      }
    } catch (error) {
      throw new Error(error);
    }
  }
}

module.exports = IoTDBHandler;
