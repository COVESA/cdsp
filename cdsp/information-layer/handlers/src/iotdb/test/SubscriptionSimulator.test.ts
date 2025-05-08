jest.mock("../config/database-params", () => ({
  ...jest.requireActual("../config/database-params"),
  databaseConfig: {
    iotdbHost: "mock-host",
    iotdbPort: 5678,
    iotdbUser: "mock-user",
    iotdbPassword: "mock-password",
    fetchSize: 2000,
    timeZoneId: "UTC",
    pollIntervalLenInSec: 15, // Mocked interval for the test
  },
}));

jest.mock('thrift', () => ({
  createConnection: jest.fn(() => ({
    on: jest.fn(),
    end: jest.fn(),
    destroy: jest.fn(),
  })),
}));

// Mock the module where transformSessionDataSet is defined
jest.mock('../utils/database-helper', () => ({
  transformSessionDataSet: jest.fn(),  // Ensure transformSessionDataSet is a mock function
}));

// Mock the module where transformSessionDataSet is defined
jest.mock('../utils/database-helper', () => ({
  transformSessionDataSet: jest.fn(),  // Ensure transformSessionDataSet is a mock function
}));

import {Subscription, SubscriptionSimulator} from '../src/SubscriptionSimulator';
import {WebSocketWithId} from '../../../../utils/database-params';
import {Session} from '../src/Session';
import {DataContentMessage, StatusMessage, SubscribeMessageType, UnsubscribeMessageType} from '../../../../router/utils/NewMessage';

describe('SubscriptionSimulator', () => {
  let simulator: SubscriptionSimulator;
  let mockSession: Session;
  let mockWebSocket: WebSocketWithId;
  let mockSubscribeMessageVIN1: SubscribeMessageType;
  let mockSubscribeMessageVIN2: SubscribeMessageType;
  let mockUnsubscribeMessageVIN1: UnsubscribeMessageType;
  let mockUnsubscribeMessageVIN2: UnsubscribeMessageType;
  let sendMessageToClientMock: jest.Mock;
  let sendAlreadySubscribedErrorMsg: jest.Mock;

  beforeEach(() => {
    mockSession = new Session();

    const createStatusMessageMock = jest.fn<
      StatusMessage, // Return type
      [number, string, string?] // Parameters: code, message, optional requestId
    >((code, statusMessage, requestId) => ({
      type: "status",
      code,
      message: statusMessage,
      ...(requestId && {requestId}),
      timestamp: {seconds: 1715253322, nanos: 123000000}, // Mocked timestamp
    }));

    sendAlreadySubscribedErrorMsg = jest.fn();
    sendMessageToClientMock = jest.fn();
    simulator = new SubscriptionSimulator(sendMessageToClientMock, jest.fn(), createStatusMessageMock, sendAlreadySubscribedErrorMsg);

    mockWebSocket = {id: 'ws1'} as WebSocketWithId;
    mockSubscribeMessageVIN1 = {instance: 'VIN_1'} as SubscribeMessageType;
    mockSubscribeMessageVIN2 = {instance: 'VIN_2'} as SubscribeMessageType;
    mockUnsubscribeMessageVIN1 = {instance: 'VIN_1'} as UnsubscribeMessageType;
    mockUnsubscribeMessageVIN2 = {instance: 'VIN_2'} as UnsubscribeMessageType;
    // Create already a subscription for 2 data points
    let subscription = {vin: "VIN_1", dataPoints: new Set<string>(['datapoint1', 'datapoint2'])};
    simulator['websocketToSubscriptionsMap'].set(mockWebSocket, [subscription]);

  });

  afterEach(() => {
    jest.clearAllMocks();
    if (simulator["intervalId"]) {
      clearInterval(simulator["intervalId"]);
      simulator["intervalId"] = null; // Reset intervalId
    }
    jest.clearAllTimers(); // Clear any remaining active timers

    process.removeAllListeners("exit");
    process.removeAllListeners("SIGINT");
  });

  /*
   * subscribe
   */

  test('Respond with BAD_REQUEST if subscription (vin & datapoints) with same webSocket already exists', () => {
    // Act: Call subscribe with the existing WebSocket and message key
    simulator.subscribe(mockSubscribeMessageVIN1, mockWebSocket, ["datapoint1"]);

    // Assert: Check that sendMessageToClient was called with the expected error message
    expect(sendAlreadySubscribedErrorMsg).toHaveBeenCalledWith(
      mockWebSocket,
      mockSubscribeMessageVIN1.instance,
      ["datapoint1"]
    );
  });

  test('Creates a new subscription if client is not subscribed to provided VIN', () => {
    // Act: Call subscribe with the existing WebSocket and another message key
    simulator.subscribe(mockSubscribeMessageVIN2, mockWebSocket, ['datapoint1', 'datapoint2']);

    // Assert: Check that a new subscription was added
    expect(simulator['websocketToSubscriptionsMap'].size).toBe(1);
    expect(simulator['websocketToSubscriptionsMap'].get(mockWebSocket)!.length).toBe(2);
    expect(simulator['websocketToSubscriptionsMap'].get(mockWebSocket)!.find(
      subscription => subscription.vin === mockSubscribeMessageVIN2.instance
    )?.dataPoints.size).toBe(2);
  });

  test('Creates a new subscription if client is not subscribed to provided datapoint', () => {
    // Act: Call subscribe with the existing WebSocket and another message key
    simulator.subscribe(mockSubscribeMessageVIN1, mockWebSocket, ['datapointNew', 'datapoint2']);

    // Assert: Check that a new subscription was added
    expect(simulator['websocketToSubscriptionsMap'].size).toBe(1);
    expect(simulator['websocketToSubscriptionsMap'].get(mockWebSocket)!.length).toBe(1);
    expect(simulator['websocketToSubscriptionsMap'].get(mockWebSocket)!.find(
      subscription => subscription.vin === mockSubscribeMessageVIN1.instance
    )?.dataPoints.size).toBe(3);
  });

  test('starts a periodic database listener if no intervalId is set', () => {
    // Arrange: Ensure there is no active interval
    simulator['intervalId'] = null;

    // Act: Call subscribe to trigger the timer start
    simulator.subscribe(mockSubscribeMessageVIN2, mockWebSocket, ['datapoint1', 'datapoint2']);

    // Assert: Check that intervalId is no longer null
    expect(simulator['intervalId']).not.toBeNull();
  });

  /*
   * unsubscribe
   */

  test('remove datapoint from already available subscription', () => {
    // Act: Call subscribe with existing WebSocket and message key
    simulator.unsubscribe(mockUnsubscribeMessageVIN1, mockWebSocket, ["datapoint1"]);

    // Assert: Check that the subscription exists with only the other websocket left
    expect(simulator['websocketToSubscriptionsMap'].size).toBe(1);
    expect(simulator['websocketToSubscriptionsMap'].get(mockWebSocket)!.length).toBe(1);
    expect(simulator['websocketToSubscriptionsMap'].get(mockWebSocket)!.find(
      subscription => subscription.vin === mockSubscribeMessageVIN1.instance
    )?.dataPoints.size).toBe(1);
    expect(simulator['websocketToSubscriptionsMap'].get(mockWebSocket)!.find(
      subscription => subscription.vin === mockSubscribeMessageVIN1.instance
    )?.dataPoints.has('datapoint2')).toBeTruthy();
  });


  test('remove subscription if unsubscribing to all datapoints', () => {
    // Act: Call unsubscribe with existing WebSocket and message key
    simulator.unsubscribe(mockUnsubscribeMessageVIN1, mockWebSocket, ['datapoint1', 'datapoint2']);

    // Assert: Check that the subscription has been removed
    expect(simulator['websocketToSubscriptionsMap'].size).toBe(0);
  });

  test('calls sendMessageToClient if unsubscribe is done on non-existing subscription', () => {
    // Act: Call unsubscribe with existing WebSocket and non-existing message key
    simulator.unsubscribe(mockUnsubscribeMessageVIN1, mockWebSocket, ['datapointUnknown']);

    // Assert: Check that sendMessageToClient was called with the expected error message
    expect(sendMessageToClientMock).toHaveBeenCalledWith(
      mockWebSocket,
      expect.objectContaining({
        type: 'status',
        code: 404,
        message: expect.stringContaining('Cannot unsubscribe')
      })
    );
  });

  test('stops the timer if there is no subscription left', () => {
    // Arrange: create a subscription to start the timer
    simulator.subscribe(mockSubscribeMessageVIN1, mockWebSocket, ['datapointNew']);
    expect(simulator['intervalId']).not.toBeNull();

    // Act: Call unsubscribe to trigger the timer removal
    simulator.unsubscribe(mockUnsubscribeMessageVIN1, mockWebSocket, ['datapoint1', 'datapoint2', 'datapointNew']);

    // Assert: Check that intervalId is null
    expect(simulator['intervalId']).toBeNull();
  });

  /*
   * notifyDatabaseChanges
   */

  test('timeIntervalUpperLimit and timeIntervalLowerLimit form a series', async () => {
    simulator["timeIntervalLowerLimit"] = Date.now();

    // Wait a little to ensure a time difference in the next call
    await new Promise(resolve => setTimeout(resolve, 10));

    // Spy on `checkForChanges`
    let timeIntervals: { lower: number; upper: number }[] = [];

    (jest.spyOn(simulator as any, 'checkForChanges') as jest.Mock)
      .mockImplementation((id: string, dataPoints: Set<string>, upperLimit: number) => {
        timeIntervals.push({
          lower: simulator["timeIntervalLowerLimit"]!,  // Current lower limit
          upper: upperLimit,                            // Passed upper limit
        });
        return Promise.resolve(undefined); // Simulate checkForChanges returning no updates
      });

    // First call
    await (simulator as any).notifyDatabaseChanges();

    // Wait a little to ensure a time difference in the next call
    await new Promise(resolve => setTimeout(resolve, 10));

    // Second call
    await (simulator as any).notifyDatabaseChanges();

    // Expect two interval records (one for each call)
    expect(timeIntervals.length).toBe(2);

    const {lower: lower1, upper: upper1} = timeIntervals[0];
    const {lower: lower2, upper: upper2} = timeIntervals[1];

    // Assertions
    expect(upper1).toBeGreaterThan(lower1); // First interval: upper > lower
    expect(upper2).toBeGreaterThan(lower2); // Second interval: upper > lower
    expect(lower2).toBe(upper1);            // Second interval's lower is the first interval's upper
  });

  test('calls sendMessageToClient if there is a matching subscription and update', async () => {
    simulator["timeIntervalLowerLimit"] = Date.now();

    // Mock checkForChanges to return an update message
    jest.spyOn(simulator as any, 'checkForChanges').mockResolvedValueOnce({
      type: "data",
      instance: 'VIN_1',
      schema: 'Vehicle',
      data: [{"CurrentLocation.Latitude": 55, "CurrentLocation.Longitude": 66}]
    } as DataContentMessage); // Return type MessageWithNodes

    // Call notifyDatabaseChanges
    await (simulator as any).notifyDatabaseChanges();

    // Assert that sendMessageToClient was called with the matching websocket and update message
    expect(sendMessageToClientMock).toHaveBeenCalledWith(
      {id: 'ws1'}, // WebSocket in the subscription
      expect.objectContaining(
        {
          instance: "VIN_1",
          schema: 'Vehicle',
          type: 'data',
          data: [{
            "CurrentLocation.Latitude": 55,
            "CurrentLocation.Longitude": 66
          }],
        })
    );
  });
});
