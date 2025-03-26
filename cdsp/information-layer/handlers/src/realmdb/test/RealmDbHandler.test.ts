import Realm from "realm";
import {RealmDBHandler} from "../src/RealmDbHandler";
import {SubscribeMessageType} from "../../../../router/utils/NewMessage";
import {WebSocketWithId} from "../../../../utils/database-params";

jest.mock("realm"); // Mock Realm
jest.mock("../config/database-params", () => ({
  databaseConfig: {
    path: "mocked.realm",
    schema: [{name: "MediaElement", properties: {vin: "string"}}],
    schemaVersion: 1, // Add if required
  },
}));
describe("RealmDBHandler", () => {
  let realmHandler: RealmDBHandler;
  let mockRealmInstance: Realm;
  let mockSubscribeMsgVehicle: SubscribeMessageType;
  let mockSubscribeMsgVehicleCurrentLocation: SubscribeMessageType;
  let mockRealmObject: Realm.Object;
  let mockWebSocket: WebSocketWithId;

  beforeEach(() => {
    jest.clearAllMocks();

    mockWebSocket = {id: "client1", send: jest.fn()};
    mockSubscribeMsgVehicle = {instance: 'VIN_1', path: "Vehicle"} as SubscribeMessageType;
    mockSubscribeMsgVehicleCurrentLocation = {instance: "VIN_1", path: "Vehicle_CurrentLocation"} as SubscribeMessageType;

    // Mock Realm Instance
    mockRealmInstance = {
      objects: jest.fn(() => ({
        filtered: jest.fn(() => [{id: "123", vin: "VIN_1"}]), // Mock object
      })),
      write: jest.fn((callback) => callback()),
      create: jest.fn(),
      schema: [{name: "MediaElement", properties: {vin: "string"}}],
    } as unknown as Realm;

    // Initialize RealmDBHandler with mocked realm
    realmHandler = new RealmDBHandler();
    (realmHandler as any).realm = mockRealmInstance; // Inject mocked realm
    // Create a mock Realm object

    mockRealmObject = {
      addListener: jest.fn(), // Mock `addListener`
      removeListener: jest.fn(),
    } as unknown as Realm.Object;

    // Access the private mediaElementCache and set a mock object
    (realmHandler as any).mediaElementCache = new Map<string, Realm.Object>();
    (realmHandler as any).mediaElementCache.set("VIN_1", mockRealmObject);

    // Mock the method getKnownDatapointsByPrefix
    jest.spyOn(realmHandler, "getKnownDatapointsByPrefix").mockImplementation((prefix: string) => {
      switch (prefix) {
        case "Vehicle":
          return ["Vehicle_Speed", "Vehicle_CurrentLocation_Latitude", "Vehicle_CurrentLocation_Longitude"];
        case "Vehicle_CurrentLocation":
          return ["Vehicle_CurrentLocation_Latitude", "Vehicle_CurrentLocation_Longitude"];
        default:
          return [];
      }
    });

    // Mock sendRequestedDataPointsNotFoundErrorMsg
    jest.spyOn(realmHandler as any, "sendRequestedDataPointsNotFoundErrorMsg").mockImplementation(jest.fn());

    // Mock sendAlreadySubscribedErrorMsg
    jest.spyOn(realmHandler as any, "sendAlreadySubscribedErrorMsg").mockImplementation(jest.fn());

    // Mock sendMessageToClient
    jest.spyOn(realmHandler as any, "sendMessageToClient").mockImplementation(jest.fn());
  });


  test("should update listener when data points are unsubscribed from an existing subscription", async () => {
    expect((realmHandler as any).instanceToSubscriptionMap.has(mockSubscribeMsgVehicle.instance)).toBe(false);

    await (realmHandler as any).subscribe({instance: 'VIN_1', path: "Vehicle"}, mockWebSocket);

    expect(mockRealmObject.addListener).toHaveBeenCalledTimes(1)
    expect((realmHandler as any).instanceToSubscriptionMap.has(mockSubscribeMsgVehicle.instance)).toBe(true);
    expect((realmHandler as any).instanceToSubscriptionMap.get(mockSubscribeMsgVehicle.instance).dataPoints.size).toBe(3);
    expect((realmHandler as any).instanceToSubscriptionMap.get(mockSubscribeMsgVehicle.instance).dataPoints.has("Vehicle_Speed")).toBe(true);
    expect((realmHandler as any).instanceToSubscriptionMap.get(mockSubscribeMsgVehicle.instance).dataPoints.has("Vehicle_CurrentLocation_Latitude")).toBe(true);
    expect((realmHandler as any).instanceToSubscriptionMap.get(mockSubscribeMsgVehicle.instance).dataPoints.has("Vehicle_CurrentLocation_Longitude")).toBe(true);

    await (realmHandler as any).unsubscribe({instance: 'VIN_1', path: "Vehicle_CurrentLocation"}, mockWebSocket);
    expect(mockRealmObject.addListener).toHaveBeenCalledTimes(2)
    expect(mockRealmObject.removeListener).toHaveBeenCalledTimes(1)

    expect((realmHandler as any).instanceToSubscriptionMap.has(mockSubscribeMsgVehicle.instance)).toBe(true);
    expect((realmHandler as any).instanceToSubscriptionMap.get(mockSubscribeMsgVehicle.instance).dataPoints.size).toBe(1);
    expect((realmHandler as any).instanceToSubscriptionMap.get(mockSubscribeMsgVehicle.instance).dataPoints.has("Vehicle_Speed")).toBe(true);

    await (realmHandler as any).unsubscribe({instance: 'VIN_1', path: "Vehicle"}, mockWebSocket);
    expect(mockRealmObject.removeListener).toHaveBeenCalledTimes(2)
    expect((realmHandler as any).instanceToSubscriptionMap.has(mockSubscribeMsgVehicle.instance)).toBe(false);
  });

  test("should update listener when new data points are added to an existing subscription", async () => {
    await (realmHandler as any).subscribe(mockSubscribeMsgVehicleCurrentLocation, mockWebSocket);
    expect((realmHandler as any).instanceToSubscriptionMap.get(mockSubscribeMsgVehicle.instance).dataPoints.size).toBe(2);

    // Subscribe to a different path which should add new data points
    await (realmHandler as any).subscribe(mockSubscribeMsgVehicle, mockWebSocket);
    expect((realmHandler as any).instanceToSubscriptionMap.get(mockSubscribeMsgVehicle.instance).dataPoints.size).toBe(3);
    expect(mockRealmObject.addListener).toHaveBeenCalledTimes(2)
    expect(mockRealmObject.removeListener).toHaveBeenCalledTimes(1)
  });


  test("should not subscribe if no data points exist", async () => {
    jest.spyOn(realmHandler, "getKnownDatapointsByPrefix").mockReturnValueOnce([]);

    await (realmHandler as any).subscribe({instance: "VIN_1", path: "InvalidPath"}, mockWebSocket);

    expect((realmHandler as any).sendRequestedDataPointsNotFoundErrorMsg).toHaveBeenCalledWith(mockWebSocket, "InvalidPath");
    expect((realmHandler as any).instanceToSubscriptionMap.has("VIN_1")).toBe(false);
    expect(mockRealmObject.addListener).toHaveBeenCalledTimes(0)
    expect(mockRealmObject.removeListener).toHaveBeenCalledTimes(0)
  });

  test("should not subscribe if all data points are already subscribed", async () => {
    await (realmHandler as any).subscribe(mockSubscribeMsgVehicle, mockWebSocket);
    await (realmHandler as any).subscribe(mockSubscribeMsgVehicle, mockWebSocket);

    expect((realmHandler as any).sendAlreadySubscribedErrorMsg).toHaveBeenCalledWith(mockWebSocket, "VIN_1", [
      "Vehicle_Speed",
      "Vehicle_CurrentLocation_Latitude",
      "Vehicle_CurrentLocation_Longitude",
    ]);
    expect(mockRealmObject.addListener).toHaveBeenCalledTimes(1)
    expect(mockRealmObject.removeListener).toHaveBeenCalledTimes(0)
  });


  test("should not unsubscribe if no data points exist", async () => {
    jest.spyOn(realmHandler, "getKnownDatapointsByPrefix").mockReturnValueOnce([]);

    await (realmHandler as any).unsubscribe({instance: "VIN_1", path: "InvalidPath"}, mockWebSocket);

    expect((realmHandler as any).sendRequestedDataPointsNotFoundErrorMsg).toHaveBeenCalledWith(mockWebSocket, "InvalidPath");
    expect((realmHandler as any).instanceToSubscriptionMap.has("VIN_1")).toBe(false);
    expect(mockRealmObject.addListener).toHaveBeenCalledTimes(0)
    expect(mockRealmObject.removeListener).toHaveBeenCalledTimes(0)
  });

  test("should not unsubscribe if no subscription available", async () => {
    await (realmHandler as any).unsubscribe(mockSubscribeMsgVehicle, mockWebSocket);

    expect((realmHandler as any).sendMessageToClient).lastCalledWith(
      mockWebSocket,
      expect.objectContaining({
        code: 404,
        message: "Cannot unsubscribe. No subscription for instance 'VIN_1'",
        type: "status",
        timestamp: expect.objectContaining({
          seconds: expect.any(Number),
          nanos: expect.any(Number),
        }),
      })
    );
  }); 
  test("should not unsubscribe if all data points are not subscribed", async () => {
    await (realmHandler as any).subscribe(mockSubscribeMsgVehicle, mockWebSocket);

    jest.spyOn(realmHandler, "getKnownDatapointsByPrefix").mockReturnValueOnce(["newDatapoint"]);
    await (realmHandler as any).unsubscribe(mockSubscribeMsgVehicle, mockWebSocket);

    expect((realmHandler as any).sendMessageToClient).lastCalledWith(
      mockWebSocket,
      expect.objectContaining({
        code: 404,
        message: expect.stringContaining("Cannot unsubscribe. No subscription for instance 'VIN_1' and datapoints ["),
        type: "status",
        timestamp: expect.objectContaining({
          seconds: expect.any(Number),
          nanos: expect.any(Number),
        }),
      })
    );
  });

});
