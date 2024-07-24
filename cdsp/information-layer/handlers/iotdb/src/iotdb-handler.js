const Thrift = require("thrift");
const Handler = require("../../handler");
const IClientRPCService = require("../gen-nodejs/IClientRPCService");
const ttypes = require("../gen-nodejs/client_types");
const config = require("../config/config");
const { Session } = require("inspector");
const sendMessageToClient = (ws, message) => {
  ws.send(JSON.stringify(message));
};

class IoTDBHandler extends Handler {
  constructor() {
    super();
    this.connection = null;
    this.client = null;
    this.sendMessageToClients = null;
    this.sessionId = null;
    this.statementId = 0;
  }

  async authenticateAndConnect(sendMessageToClients) {
    try {
      this.sendMessageToClients = sendMessageToClients;

      this.connection = Thrift.createConnection(
        config.iotdbHost,
        config.iotdbPort,
        {
          transport: Thrift.TFramedTransport,
          protocol: Thrift.TBinaryProtocol,
        }
      );

      this.client = Thrift.createClient(IClientRPCService, this.connection);

      this.connection.on("error", (err) => {
        console.error("Thrift connection error:", err);
      });

      console.log("Successfully connected to IoTDB using Thrift");

      this.open_session();
    } catch (error) {
      console.error("Failed to authenticate with IoTDB:", error);
    }
  }

  async open_session() {
    try {
      const openSessionReq = new ttypes.TSOpenSessionReq({
        username: config.iotdbUser,
        password: config.iotdbPassword,
        client_protocol: ttypes.TSProtocolVersion.IOTDB_SERVICE_PROTOCOL_V3, // Set the appropriate protocol version
        zoneId: config.timeZone,
      });

      const resp = await this.client.openSession(openSessionReq);

      this.sessionId = resp.sessionId;
      console.log(
        "Successfully authenticated with IoTDB, session ID:",
        this.sessionId
      );
    } catch (error) {
      console.error("Failed starting session with IoTDB:", error);
    }
  }
}

module.exports = IoTDBHandler;
