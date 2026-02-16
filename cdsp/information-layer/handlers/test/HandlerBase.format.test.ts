import { HandlerBase, QueryResult } from "../src/HandlerBase";
import {
  GetMessageType,
  NewMessageType,
  DataContentMessage,
} from "../../router/utils/NewMessage";
import { WebSocketWithId } from "../../utils/database-params";

/**
 * Test HandlerBase formatting and root options
 */

// Mock implementation of HandlerBase for testing
class TestHandler extends HandlerBase {
  constructor(sendMessageFn: any) {
    super(sendMessageFn);
  }

  getKnownDatapointsByPrefix(prefix: string): string[] {
    // Mock implementation - return expected datapoints based on prefix
    if (prefix.includes("CurrentLocation")) {
      return [
        "Vehicle_CurrentLocation_Latitude",
        "Vehicle_CurrentLocation_Longitude",
      ];
    } else if (prefix.includes("Speed")) {
      return ["Vehicle_Speed"];
    } else if (prefix === "Vehicle" || prefix === "") {
      return [
        "Vehicle_CurrentLocation_Latitude",
        "Vehicle_CurrentLocation_Longitude",
        "Vehicle_Speed",
      ];
    }
    return [];
  }

  async getDataPointsFromDB(
    dataPoints: string[],
    vin: string,
  ): Promise<QueryResult> {
    // Mock DB response with test data
    const dataPointsArray = dataPoints.map((dp) => {
      if (dp === "Vehicle_CurrentLocation_Latitude") {
        return { name: "Vehicle.CurrentLocation.Latitude", value: 60 };
      } else if (dp === "Vehicle_CurrentLocation_Longitude") {
        return { name: "Vehicle.CurrentLocation.Longitude", value: 50 };
      } else if (dp === "Vehicle_Speed") {
        return { name: "Vehicle.Speed", value: 100 };
      }
      return { name: dp, value: null };
    });

    const metadataArray = dataPoints.map((dp) => {
      if (dp === "Vehicle_CurrentLocation_Latitude") {
        return {
          name: "Vehicle.CurrentLocation.Latitude",
          value: {
            timestamps: {
              received: { seconds: 1770028431, nanos: 250057547 },
            },
          },
        };
      } else if (dp === "Vehicle_CurrentLocation_Longitude") {
        return {
          name: "Vehicle.CurrentLocation.Longitude",
          value: {
            timestamps: {
              received: { seconds: 1770028431, nanos: 250057547 },
            },
          },
        };
      } else if (dp === "Vehicle_Speed") {
        return {
          name: "Vehicle.Speed",
          value: {
            timestamps: {
              received: { seconds: 1770110638, nanos: 660818568 },
            },
          },
        };
      }
      return { name: dp, value: {} };
    });

    return {
      success: true,
      dataPoints: dataPointsArray,
      metadata: metadataArray,
    };
  }
}

describe("HandlerBase - Format and Root Options", () => {
  let handler: TestHandler;
  let capturedMessage: DataContentMessage | null;
  let mockWebSocket: WebSocketWithId;

  beforeEach(() => {
    capturedMessage = null;
    handler = new TestHandler((ws: WebSocketWithId, message: any) => {
      if (message.type === "data") {
        capturedMessage = message as DataContentMessage;
      }
    });
    mockWebSocket = { id: "test-ws" } as WebSocketWithId;
  });

  describe("path contains only root Vehicle", () => {
    it("should format flat + relative", async () => {
      const message: GetMessageType = {
        type: NewMessageType.Get,
        instance: "SMT123JN12J123456",
        path: "Vehicle",
        requestId: "2",
        root: "relative",
        format: "flat",
      };

      await handler.get(message, mockWebSocket);

      expect(capturedMessage).not.toBeNull();
      expect(capturedMessage?.data).toEqual({
        "CurrentLocation.Latitude": 60,
        "CurrentLocation.Longitude": 50,
        Speed: 100,
      });
      expect(capturedMessage?.metadata).toEqual({
        "CurrentLocation.Latitude": {
          timestamps: {
            received: { seconds: 1770028431, nanos: 250057547 },
          },
        },
        "CurrentLocation.Longitude": {
          timestamps: {
            received: { seconds: 1770028431, nanos: 250057547 },
          },
        },
        Speed: {
          timestamps: {
            received: { seconds: 1770110638, nanos: 660818568 },
          },
        },
      });
    });

    it("should format flat + absolute", async () => {
      const message: GetMessageType = {
        type: NewMessageType.Get,
        instance: "SMT123JN12J123456",
        path: "Vehicle",
        requestId: "2",
        root: "absolute",
        format: "flat",
      };

      await handler.get(message, mockWebSocket);

      expect(capturedMessage).not.toBeNull();
      expect(capturedMessage?.data).toEqual({
        "CurrentLocation.Latitude": 60,
        "CurrentLocation.Longitude": 50,
        Speed: 100,
      });
    });

    it("should format nested + relative", async () => {
      const message: GetMessageType = {
        type: NewMessageType.Get,
        instance: "SMT123JN12J123456",
        path: "Vehicle",
        requestId: "2",
        root: "relative",
        format: "nested",
      };

      await handler.get(message, mockWebSocket);

      expect(capturedMessage).not.toBeNull();
      expect(capturedMessage?.data).toEqual({
        CurrentLocation: {
          Latitude: 60,
          Longitude: 50,
        },
        Speed: 100,
      });
    });

    it("should format nested + absolute", async () => {
      const message: GetMessageType = {
        type: NewMessageType.Get,
        instance: "SMT123JN12J123456",
        path: "Vehicle",
        requestId: "2",
        root: "absolute",
        format: "nested",
      };

      await handler.get(message, mockWebSocket);

      expect(capturedMessage).not.toBeNull();
      expect(capturedMessage?.data).toEqual({
        CurrentLocation: {
          Latitude: 60,
          Longitude: 50,
        },
        Speed: 100,
      });
    });
  });

  describe("Path contains  Vehicle_CurrentLocation", () => {
    it("should format flat + relative - removes CurrentLocation from keys", async () => {
      const message: GetMessageType = {
        type: NewMessageType.Get,
        instance: "SMT123JN12J123456",
        path: "Vehicle_CurrentLocation",
        requestId: "2",
        root: "relative",
        format: "flat",
      };

      await handler.get(message, mockWebSocket);

      expect(capturedMessage).not.toBeNull();
      expect(capturedMessage?.data).toEqual({
        Latitude: 60,
        Longitude: 50,
      });
      expect(capturedMessage?.metadata).toEqual({
        Latitude: {
          timestamps: {
            received: { seconds: 1770028431, nanos: 250057547 },
          },
        },
        Longitude: {
          timestamps: {
            received: { seconds: 1770028431, nanos: 250057547 },
          },
        },
      });
    });

    it("should format flat + absolute - keeps full path", async () => {
      const message: GetMessageType = {
        type: NewMessageType.Get,
        instance: "SMT123JN12J123456",
        path: "Vehicle_CurrentLocation",
        requestId: "2",
        root: "absolute",
        format: "flat",
      };

      await handler.get(message, mockWebSocket);

      expect(capturedMessage).not.toBeNull();
      expect(capturedMessage?.data).toEqual({
        "CurrentLocation.Latitude": 60,
        "CurrentLocation.Longitude": 50,
      });
      expect(capturedMessage?.metadata).toEqual({
        "CurrentLocation.Latitude": {
          timestamps: {
            received: { seconds: 1770028431, nanos: 250057547 },
          },
        },
        "CurrentLocation.Longitude": {
          timestamps: {
            received: { seconds: 1770028431, nanos: 250057547 },
          },
        },
      });
    });

    it("should format nested + relative - direct object without CurrentLocation", async () => {
      const message: GetMessageType = {
        type: NewMessageType.Get,
        instance: "SMT123JN12J123456",
        path: "Vehicle_CurrentLocation",
        requestId: "2",
        root: "relative",
        format: "nested",
      };

      await handler.get(message, mockWebSocket);

      expect(capturedMessage).not.toBeNull();
      expect(capturedMessage?.data).toEqual({
        Latitude: 60,
        Longitude: 50,
      });
    });

    it("should format nested + absolute - includes CurrentLocation hierarchy", async () => {
      const message: GetMessageType = {
        type: NewMessageType.Get,
        instance: "SMT123JN12J123456",
        path: "Vehicle_CurrentLocation",
        requestId: "2",
        root: "absolute",
        format: "nested",
      };

      await handler.get(message, mockWebSocket);

      expect(capturedMessage).not.toBeNull();
      expect(capturedMessage?.data).toEqual({
        CurrentLocation: {
          Latitude: 60,
          Longitude: 50,
        },
      });
    });
  });

  describe("Path contains CurrentLocation.Latitude (leaf node)", () => {
    beforeEach(() => {
      // Override mock for single datapoint
      handler.getKnownDatapointsByPrefix = jest
        .fn()
        .mockReturnValue(["Vehicle_CurrentLocation_Latitude"]);
    });

    it("should format flat + relative - empty string key for exact match", async () => {
      const message: GetMessageType = {
        type: NewMessageType.Get,
        instance: "SMT123JN12J123456",
        path: "Vehicle_CurrentLocation_Latitude",
        requestId: "2",
        root: "relative",
        format: "flat",
      };

      await handler.get(message, mockWebSocket);

      expect(capturedMessage).not.toBeNull();
      expect(capturedMessage?.data).toEqual({
        "": 60,
      });
      expect(capturedMessage?.metadata).toEqual({
        "": {
          timestamps: {
            received: { seconds: 1770028431, nanos: 250057547 },
          },
        },
      });
    });

    it("should format flat + absolute - full path as key", async () => {
      const message: GetMessageType = {
        type: NewMessageType.Get,
        instance: "SMT123JN12J123456",
        path: "Vehicle_CurrentLocation_Latitude",
        requestId: "2",
        root: "absolute",
        format: "flat",
      };

      await handler.get(message, mockWebSocket);

      expect(capturedMessage).not.toBeNull();
      expect(capturedMessage?.data).toEqual({
        "CurrentLocation.Latitude": 60,
      });
      expect(capturedMessage?.metadata).toEqual({
        "CurrentLocation.Latitude": {
          timestamps: {
            received: { seconds: 1770028431, nanos: 250057547 },
          },
        },
      });
    });

    it("should format nested + relative - direct value (no object wrapper)", async () => {
      const message: GetMessageType = {
        type: NewMessageType.Get,
        instance: "SMT123JN12J123456",
        path: "Vehicle_CurrentLocation_Latitude",
        requestId: "2",
        root: "relative",
        format: "nested",
      };

      await handler.get(message, mockWebSocket);

      expect(capturedMessage).not.toBeNull();
      // Special case: nested + relative for exact match returns value directly
      expect(capturedMessage?.data).toBe(60);
      expect(capturedMessage?.metadata).toEqual({
        "": {
          timestamps: {
            received: { seconds: 1770028431, nanos: 250057547 },
          },
        },
      });
    });

    it("should format nested + absolute - builds full hierarchy", async () => {
      const message: GetMessageType = {
        type: NewMessageType.Get,
        instance: "SMT123JN12J123456",
        path: "Vehicle_CurrentLocation_Latitude",
        requestId: "2",
        root: "absolute",
        format: "nested",
      };

      await handler.get(message, mockWebSocket);

      expect(capturedMessage).not.toBeNull();
      expect(capturedMessage?.data).toEqual({
        CurrentLocation: {
          Latitude: 60,
        },
      });
      expect(capturedMessage?.metadata).toEqual({
        "CurrentLocation.Latitude": {
          timestamps: {
            received: { seconds: 1770028431, nanos: 250057547 },
          },
        },
      });
    });
  });
});
