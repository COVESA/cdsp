import {IoTDBHandler} from "../src/IoTDBHandler";
import {Session} from "../src/Session";
import {SessionDataSet} from "../utils/SessionDataSet";
import {WebSocketWithId} from "../../../../utils/database-params";
import {NewMessageType, SetMessageType} from "../../../../router/utils/NewMessage";
import {SupportedMessageDataTypes} from "../utils/iotdb-constants";
import {STATUS_SUCCESS} from "../../../../router/utils/ErrorMessage";

jest.mock("../src/Session");
jest.mock("../src/SubscriptionSimulator");
jest.mock("../utils/database-helper", () => ({
  transformSessionDataSet: jest.fn(() => [
    [{name: "datapoint", value: 1}], // Mocked data
    [{name: "metadata", value: 2}]   // Mocked metadata
  ])
}));
jest.mock("../config/database-params", () => ({
  databaseConfig: {},
  databaseParams: {VSS: {databaseName: "test_db", dataPointId: "Vehicle_VehicleIdentification_VIN"}}
}));

describe("IoTDBHandler", () => {
  let handler: IoTDBHandler;
  let mockSession: jest.Mocked<Session>;
  let mockWebSocket: jest.Mocked<WebSocketWithId>;

  beforeEach(() => {
    mockSession = new Session() as jest.Mocked<Session>;
    mockSession.authenticateAndConnect = jest.fn();
    mockSession.executeQueryStatement = jest.fn();

    handler = new IoTDBHandler();
    (handler as any).session = mockSession; // Inject mocked session
    mockWebSocket = {id: "test-socket"} as jest.Mocked<WebSocketWithId>;
    handler["sendMessageToClient"] = jest.fn();
  });

  test("should authenticate and connect to IoTDB", async () => {
    await handler.authenticateAndConnect();
    expect(mockSession.authenticateAndConnect).toHaveBeenCalled();
  });

  test("should query data points and metadata from IoTDB", async () => {
    const mockDataPoints = ["Temperature", "Speed"];
    const vin = "TEST_VIN";
    mockSession.executeQueryStatement.mockResolvedValueOnce(new SessionDataSet([], [], {}, 0, {}, 0, {}, {}, false));

    const result = await handler.getDataPointsFromDB(mockDataPoints, vin);
    // Ensure the metadata timeseries have been requested together with the data points
    expect(mockSession.executeQueryStatement).toHaveBeenCalledWith(
      "SELECT Temperature, Speed, Temperature_Metadata, Speed_Metadata\n" +
      "                 FROM test_db\n" +
      "                 WHERE Vehicle_VehicleIdentification_VIN = 'TEST_VIN'\n" +
      "                 ORDER BY Time ASC"
    );
    expect(result.success).toBe(true);
  });

  test("should insert data into IoTDB", async () => {
    const mockSetMessage: SetMessageType = {
      type: NewMessageType.Set,
      instance: "TEST_VIN",
      path: "Vehicle",
      data: {Speed: 60, Temperature: 42.42},
      metadata: {Speed: {unit: "km/h"}},
    };
    (handler as any).dataPointsSchema = {
      "Vehicle_Speed": SupportedMessageDataTypes.int16,
      "Vehicle_Temperature": SupportedMessageDataTypes.float,
      "Vehicle_VehicleIdentification_VIN": SupportedMessageDataTypes.string
    };
    mockSession.getSessionId = jest.fn().mockReturnValue("mockSessionId");
    mockSession.getClient = jest.fn().mockReturnValue({
      insertRecord: jest.fn().mockResolvedValue({status: 200}),
    });

    await (handler as any).set(mockSetMessage, mockWebSocket);

    expect(mockSession.getClient().insertRecord).toHaveBeenCalled();
    expect(handler["sendMessageToClient"]).toHaveBeenCalled();
    expect(handler["sendMessageToClient"]).toHaveBeenCalledWith(
      mockWebSocket,
      expect.objectContaining({code: STATUS_SUCCESS.OK})
    );

    // Ensure metadata is added for Speed only
    expect(mockSession.getClient().insertRecord).toHaveBeenCalledWith(
      expect.objectContaining({
        measurements: [
          "Vehicle_Speed",
          "Vehicle_Temperature",
          "Vehicle_VehicleIdentification_VIN",
          "Vehicle_Speed_Metadata"
        ],
      })
    );
  });

  test("should correctly format metadata", () => {
    const mockMessage: SetMessageType = {
      instance: "",
      type: NewMessageType.Set,
      path: "Vehicle.Speed",
      data: {},
      metadata: {VIN: "123456", Unit: "km/h"}
    };

    const result = (handler as any).extractNodesFromMetadata(mockMessage);

    expect(result).toEqual({
      "Vehicle.Speed_VIN_Metadata": JSON.stringify("123456"),
      "Vehicle.Speed_Unit_Metadata": JSON.stringify("km/h")
    });
  });

});
