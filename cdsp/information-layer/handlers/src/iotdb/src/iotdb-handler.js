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
const SessionDataSet = require("../utils/session-data-set");
const { IoTDBDataInterpreter } = require("../utils/iotdb-data-interpreter");
const { createDataPointsSchema } = require("../config/data-points-type");
const { databaseParams, databaseConfig } = require("../config/database-params");
const {
  logMessage,
  logWithColor,
  MessageType,
  COLORS,
} = require("../../../../utils/logger");
const {
  createErrorMessage,
} = require("../../../../utils/error-message-helper");

class IoTDBHandler extends Handler {
  constructor() {
    super();
    this.client = null;
    this._sendMessageToClients = null;
    this.sessionId = null;
    this.protocolVersion = TSProtocolVersion.IOTDB_SERVICE_PROTOCOL_V3;
    this.statementId = null;
    this.zoneId = databaseConfig.timeZoneId;
    this.fetchSize = databaseConfig.fetchSize;
    this.isSessionClosed = true;
    this.dataPointsSchema = null;
  }

  async authenticateAndConnect(sendMessageToClients) {
    try {
      this._sendMessageToClients = sendMessageToClients;

      const connection = thrift.createConnection(
        databaseConfig.iotdbHost,
        databaseConfig.iotdbPort,
        {
          transport: thrift.TFramedTransport,
          protocol: thrift.TBinaryProtocol,
        },
      );

      this.client = thrift.createClient(Client, connection);

      connection.on("error", (error) => {
        logMessage(
          "thrift connection error: ".concat(error),
          MessageType.ERROR,
        );
      });

      const supportedDataPoint = this._getSupportedDataPoints();
      this.dataPointsSchema = createDataPointsSchema(supportedDataPoint);

      console.info("Successfully connected to IoTDB using thrift");
    } catch (error) {
      logMessage(
        "Failed to authenticate with IoTDB: ".concat(error),
        MessageType.ERROR,
      );
    }
  }

  async read(message, ws) {
    if (this.#areNodesValid(message, ws)) {
      try {
        await this.#openSessionIfNeeded();
        const responseNodes = await this.#queryLastFields(message, ws);
        if (responseNodes.length > 0) {
          const responseMessage = this._createUpdateMessage(
            message,
            responseNodes,
          );
          this._sendMessageToClient(ws, responseMessage);
        } else {
          this._sendMessageToClient(
            ws,
            createErrorMessage(
              "read",
              404,
              `No data found with the Id: ${message.id}`,
            ),
          );
        }
      } catch (error) {
        this._sendMessageToClient(
          ws,
          createErrorMessage("read", 404, error.message),
        );
      } finally {
        await this.#closeSessionIfNeeded();
      }
    }
  }

  async write(message, ws) {
    if (this.#areNodesValid(message, ws)) {
      try {
        await this.#openSessionIfNeeded();
        const data = this.#createObjectToInsert(message);
        let measurements = [];
        let dataTypes = [];
        let values = [];

        for (const [key, value] of Object.entries(data)) {
          measurements.push(key);
          dataTypes.push(this.dataPointsSchema[key]);
          values.push(value);
        }

        const timestamp = new Date().getTime();
        const deviceId = databaseParams[message.tree].databaseName;
        const status = await this.#insertRecord(
          deviceId,
          timestamp,
          measurements,
          dataTypes,
          values,
        );

        logWithColor(
          `Record inserted to device ${deviceId}, status code: `.concat(
            JSON.stringify(status),
          ),
          COLORS.GREY,
        );

        const responseNodes = await this.#queryLastFields(message, ws);

        if (responseNodes.length) {
          const responseMessage = this._createUpdateMessage(
            message,
            responseNodes,
          );
          this._sendMessageToClient(ws, responseMessage);
        } else {
          this._sendMessageToClient(
            ws,
            createErrorMessage(
              "write",
              404,
              `No data found with the Id: ${message.id}`,
            ),
          );
        }
      } catch (error) {
        this._sendMessageToClient(
          ws,
          createErrorMessage("write", 503, `Failed writing data. ${error}`),
        );
      } finally {
        await this.#closeSessionIfNeeded();
      }
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
      username: databaseConfig.iotdbUser,
      password: databaseConfig.iotdbPassword,
      client_protocol: this.protocolVersion,
      zoneId: databaseConfig.timeZoneId,
      configuration: { version: "V_0_13" },
    });

    try {
      const resp = await this.client.openSession(openSessionReq);

      if (this.protocolVersion !== resp.serverProtocolVersion) {
        console.info(
          "Protocol differ, Client version is " +
            this.protocolVersion +
            ", but Server version is " +
            resp.serverProtocolVersion,
        );
        // version is less than 0.10
        if (resp.serverProtocolVersion === 0) {
          throw new Error("Protocol not supported.");
        }
      }

      this.sessionId = resp.sessionId;
      this.statementId = await this.client.requestStatementId(this.sessionId);
      this.isSessionClosed = false;
      console.info("Session started!");
    } catch (error) {
      logMessage(
        "Failed starting session with IoTDB: ".concat(error),
        MessageType.ERROR,
      );
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
      logMessage(
        "Error occurs when closing session at server. Maybe server is down. Error message: ".concat(
          error,
        ),
        MessageType.ERROR,
      );
    } finally {
      this.isSessionClosed = true;
      console.info("Session closed!");
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
   * Validates the nodes in a message against the schema of a media element.
   *
   * @param {Object} message - The message object containing details for the request.
   * @param {WebSocket} ws - The WebSocket object for communication.
   * @returns {boolean} - Returns true if all nodes are valid against the schema, otherwise false.
   */
  #areNodesValid(message, ws) {
    const { type } = message;

    const errorData = this._validateNodesAgainstSchema(
      message,
      this.dataPointsSchema,
    );

    if (errorData) {
      logMessage(
        `Error validating message nodes against schema: ${JSON.stringify(errorData)}`,
        MessageType.ERROR,
      );
      this._sendMessageToClient(
        ws,
        createErrorMessage(`${type}`, 404, errorData),
      );

      return false;
    }
    return true;
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
          resp.ignoreTimeStamp,
        );
      }
    } catch (error) {
      logMessage(
        "Failed executing query statement: ".concat(error),
        MessageType.ERROR,
      );
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
    isAligned = false,
  ) {
    if (
      values.length !== dataTypes.length ||
      values.length !== measurements.length
    ) {
      throw "length of data types does not equal to length of values!";
    }
    const valuesInBytes = IoTDBDataInterpreter.serializeValues(
      dataTypes,
      values,
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
   * Queries the last fields from the database based on the provided message and sends the response to the client.
   *
   * @param {Object} message - The message object containing the query details.
   * @param {WebSocket} ws - The WebSocket connection to send the response to.
   * @private
   */
  async #queryLastFields(message, ws) {
    const { id: objectId, tree } = message;
    const { databaseName, dataPointId } = databaseParams[tree];
    const fieldsToSearch = this.#extractDataPointsFromNodes(message).join(", ");
    const sql = `SELECT ${fieldsToSearch} FROM ${databaseName} WHERE ${dataPointId} = '${objectId}' ORDER BY Time ASC`;

    try {
      const sessionDataSet = await this.#executeQueryStatement(sql);

      if (!sessionDataSet || Object.keys(sessionDataSet).length === 0) {
        throw new Error("Internal error constructing read object.");
      }

      const mediaElements = [];
      while (sessionDataSet.hasNext()) {
        mediaElements.push(sessionDataSet.next());
      }

      let latestValues = {};
      mediaElements.forEach((mediaElement) => {
        // extract underscores from media element key
        const transformedMediaElement = Object.fromEntries(
          Object.entries(mediaElement).map(([key, value]) => {
            const newKey = this._transformDataPointsWithDots(key);
            return [newKey, value];
          }),
        );

        const transformedObject =
          IoTDBDataInterpreter.extractNodesFromTimeseries(
            transformedMediaElement,
            databaseName,
          );

        Object.entries(transformedObject).forEach(([key, value]) => {
          if (value !== null && !isNaN(value)) {
            latestValues[key] = value;
          }
        });
      });

      return Object.entries(latestValues).map(([name, value]) => ({
        name,
        value,
      }));
    } catch (error) {
      this._sendMessageToClient(
        ws,
        createErrorMessage("read", 503, error.message),
      );
    }
  }

  /**
   * Extracts data point names from the given message.
   *
   * This function checks if the message has a single node or multiple nodes and
   * extracts the names accordingly.
   *
   * @param {Object} message - The message containing node(s).
   * @returns {Array<string>} An array of data point names.
   */
  #extractDataPointsFromNodes(message) {
    let dataPoints = [];

    if (message.node) {
      dataPoints.push(
        this._transformDataPointsWithUnderscores(message.node.name),
      );
    } else if (message.nodes) {
      dataPoints = message.nodes.map((node) =>
        this._transformDataPointsWithUnderscores(node.name),
      );
    }
    return dataPoints;
  }

  /**
   * Creates an object to insert data in IoTDB based on the provided message.
   *
   * @param {Object} message - The message containing data to be inserted.
   * @returns {Object} The data object to be inserted.
   */
  #createObjectToInsert(message) {
    const { id, tree } = message;
    const data = { [databaseParams[tree].dataPointId]: id };
    if (message.node) {
      data[this._transformDataPointsWithUnderscores(message.node.name)] =
        message.node.value;
    } else if (message.nodes) {
      message.nodes.forEach((node) => {
        data[this._transformDataPointsWithUnderscores(node.name)] = node.value;
      });
    }
    return data;
  }
}

module.exports = IoTDBHandler;
