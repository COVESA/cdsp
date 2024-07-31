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

const sendMessageToClient = (ws, message) => {
  ws.send(JSON.stringify(message));
};

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
    const objectId = message.data.VIN;
    const sql = `SELECT * FROM root.Vehicles WHERE root.Vehicles.VIN = '${objectId}'`;
    try {
      await this.openSession();
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
        if (mediaElements.length > 0) {
          for (let i = 0; i < mediaElements.length; ++i) {
            const transformedObject =
              IoTDBDataInterpreter.extractDeviceIdFromTimeseries(
                mediaElements[i]
              );
            const response = {
              type: "read_response",
              data: transformedObject,
            };
            sendMessageToClient(ws, JSON.stringify(response));
          }
        } else {
          sendMessageToClient(
            ws,
            JSON.stringify({ error: "Object not found." })
          );
        }
      }
    } catch (error) {
      console.error("Failed to read data from IoTDB: ", error);
    } finally {
      this.closeSession();
    }
  }

  async write(message, ws) {
    let measurements = [];
    let dataTypes = [];
    let values = [];
    try {
      await this.openSession();
      Object.entries(message.data).forEach(async ([key, value]) => {
        measurements.push(key);
        dataTypes.push(MeasurementType[key]);
        values.push(value);
      });
      let deviceId = "root.Vehicles";
      const timestamp = new Date().getTime();
      const status = await this.#insertRecord(
        deviceId,
        timestamp,
        measurements,
        dataTypes,
        values
      );

      const response = `insert one record to device ${deviceId}, message: ${status.message}`;
      console.log(response);
      sendMessageToClient(ws, response);
    } catch (error) {
      console.error("Failed to write data from IoTDB: ", error);
    } finally {
      this.closeSession();
    }
  }

  /**
   * Opens a session with the IoTDB server using the provided credentials and configuration.
   */
  async openSession() {
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

  isSessionOpen() {
    return !this.isSessionClosed;
  }

  closeSession() {
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
        err
      );
    } finally {
      this.isSessionClosed = true;
      console.log("Session closed!");
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
}

module.exports = IoTDBHandler;
