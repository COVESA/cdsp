import thrift from "thrift";
import {
  TSOpenSessionReq,
  TSCloseSessionReq,
  TSProtocolVersion,
  TSExecuteStatementReq,
} from "../gen-nodejs/client_types";
import { logMessage, logError, logWithColor, COLORS } from "../../../../utils/logger";
import { databaseConfig } from "../config/database-params";
import { SessionDataSet } from "../utils/SessionDataSet";

const Client = require("../gen-nodejs/IClientRPCService");

const PROTOCOL_VERSION: number = TSProtocolVersion.IOTDB_SERVICE_PROTOCOL_V3;

export class Session {
  private client: any;
  private sessionId: number | null = null;      
  private statementId: number | null = null;
  private fetchSize: number;

  constructor() {
    this.fetchSize = databaseConfig?.fetchSize ?? 1000;
  }

  public getSessionId(): number | null {
      return this.sessionId;
  }
  public getClient() {
      return this.client;
  }

  async authenticateAndConnect(): Promise<void> {
    try {
      const connection = thrift.createConnection(
        databaseConfig!.iotdbHost,
        databaseConfig!.iotdbPort!,
        {
          transport: thrift.TFramedTransport,
          protocol: thrift.TBinaryProtocol,
        }
      );

      this.client = thrift.createClient(Client, connection);
      await this.openSession();

      connection.on("error", (error: Error) => {
        logError("thrift connection error", error);
      });

      process.on('exit', () => {
        if (connection) {
          connection.destroy();
        }
      });

      logMessage(`Successfully connected to IoTDB using thrift, host: ${databaseConfig!.iotdbHost} port: ${databaseConfig!.iotdbPort}`);
    } catch (error: unknown) {
      logError("Failed to authenticate with IoTDB", error);
    }
  }
  
    /**
   * Opens a session with the IoTDB server using the provided credentials and configuration.
   */
  async openSession(): Promise<void> {
      if (this.sessionId) {
        logMessage("The session is already opened, session id: " + this.sessionId);
        return;
      }
  
      const openSessionReq = new TSOpenSessionReq({
        username: databaseConfig!.iotdbUser,
        password: databaseConfig!.iotdbPassword,
        client_protocol: PROTOCOL_VERSION,
        zoneId: databaseConfig!.timeZoneId,
        configuration: { version: "V_0_13" },
      });
  
      try {
        const resp = await this.client.openSession(openSessionReq);
  
        if (PROTOCOL_VERSION !== resp.serverProtocolVersion) {
          logMessage(
            `Protocol differ, Client version is ${PROTOCOL_VERSION}, but Server version is ${resp.serverProtocolVersion}`
          );
          if (resp.serverProtocolVersion === 0) {
            throw new Error("Protocol not supported.");
          }
        }
  
        this.sessionId = resp.sessionId;
        logMessage(`session #${this.sessionId} opened`);
 
        if (!this.sessionId) {
          throw new Error("Session ID is undefined, cannot request statement ID.");
        }
  
        this.statementId = await this.client.requestStatementId(this.sessionId);
      } catch (error: unknown) {
        logError("Failed starting session with IoTDB", error);
      }
    }
  
  /**
   * Closes the current session if it is not already closed.
   */
  async closeSession(): Promise<void> {
      if (!this.sessionId) {
        logMessage("Session is already closed.");
        return;
      }
      const req = new TSCloseSessionReq({ sessionId: this.sessionId! });
  
      try {
        await this.client.closeSession(req);
      } catch (error: unknown) {
        logError(
          "Error occurs when closing session at server. Maybe server is down. Error message",
          error
        );
      } finally {
        logMessage(`session #${this.sessionId} closed`);
        this.sessionId = null;
      }
    }
  

  /**
   * Executes a SQL query statement asynchronously.
   *
   * @param sql - The SQL query statement to be executed.
   * @returns - Returns a SessionDataSet object if the query is successful,
   *                                             otherwise returns an empty object.
   * @throws - Throws an error if the session is not open.
   */
  async executeQueryStatement(
    sql: string
  ): Promise<SessionDataSet | {}> {
    try {
      if (!this.sessionId) {
        throw new Error("Session is not open. Please authenticate first.");
      }
      if (!this.statementId) {
        throw new Error("Missing statement ID");
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
      }
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
    } catch (error: unknown) {
      logError("Failed executing query statement", error);
      throw error;
    }
  }

  }
  