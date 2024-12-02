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

import { SubscriptionSimulator } from '../src//SubscriptionSimulator'; 
import { WebSocketWithId, Message, MessageBase } from '../../../utils/data-types';  
import { Session } from '../src//Session';

describe('SubscriptionSimulator', () => {
  let simulator: SubscriptionSimulator;
  let mockSession: Session;
  let mockWebSocket: WebSocketWithId;
  let mockMessage: Message;
  let mockKey: string;
  let sendMessageToClientMock: jest.Mock;

  beforeEach(() => {
    mockSession = new Session();

    const createSubscribeStatusMessageMock = jest.fn<
      MessageBase,
      ["subscribe" | "unsubscribe", Pick<Message, "id" | "tree" | "uuid">, string]
    >((type, message, status) => ({
      type, // Properly narrowed type
      id: message.id,
      tree: message.tree,
      uuid: message.uuid,
      status,
    }));

    sendMessageToClientMock = jest.fn();
    simulator = new SubscriptionSimulator(sendMessageToClientMock, jest.fn(), jest.fn());
    
    mockWebSocket = { id: 'ws1' } as WebSocketWithId;
    mockMessage = { id: 'vehicle123', tree: 'VSS' } as Message;
    mockKey = `${mockMessage.id}-${mockMessage.tree}`;
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

  test('calls sendMessageToClient if subscription with same WebSocket already exists', () => {
    // Arrange: Manually add a subscription to simulate the "already exists" condition
    simulator['subscriptions'].set(mockKey, [mockWebSocket]);

    // Act: Call subscribe with the existing WebSocket and message key
    simulator.subscribe(mockMessage, mockWebSocket);

    // Assert: Check that sendMessageToClient was called with the expected error message
    expect(sendMessageToClientMock).toHaveBeenCalledWith(
      mockWebSocket,
      expect.objectContaining({
        category: 'subscribe',
        statusCode: 400,
        message: expect.stringContaining('Subscription already done')
      })
    );
  });

  test('Creates a new subscription if no subscription with that key exists', () => {
    // Arrange: Add a subscription with another key and the same websocket
    const key = `otherId-${mockMessage.tree}`;
    simulator['subscriptions'].set(key, [mockWebSocket]);

    // Act: Call subscribe with the existing WebSocket and another message key
    simulator.subscribe(mockMessage, mockWebSocket);

    // Assert: Check that a new subscription was added
    expect(simulator['subscriptions'].size).toBe(2);
    expect(simulator['subscriptions'].has(mockKey)).toBe(true);
  });

  test('Adds a websocket to an existing subscription if websocket is not there yet', () => {
    // Arrange: Add a subscription with same key and different websocket
    const otherWebsocket = { id: 'otherWs' } as WebSocketWithId;
    simulator['subscriptions'].set(mockKey, [otherWebsocket]);

    // Act: Call subscribe with another WebSocket and existing message key
    simulator.subscribe(mockMessage, mockWebSocket);

    // Assert: Check that the existing subscription has a new websocket
    expect(simulator['subscriptions'].size).toBe(1);
    expect(simulator['subscriptions'].get(mockKey)?.length).toBe(2);
    expect(simulator['subscriptions'].get(mockKey)).toEqual(expect.arrayContaining([otherWebsocket, mockWebSocket]));
  });

  test('starts a timer if no intervalId is set', () => {
    // Arrange: Ensure there is no active interval
    simulator['intervalId'] = null;

    // Act: Call subscribe to trigger the timer start
    simulator.subscribe(mockMessage, mockWebSocket);

    // Assert: Check that intervalId is no longer null
    expect(simulator['intervalId']).not.toBeNull();
  });

  /*
   * unsubscribe
   */

  test('remove websocket from existing subscription if websocket and subscription exist', () => {
    // Arrange: Add a subscription with same key and same websocket + 1 other websocket
    const otherWebsocket = { id: 'otherWs' } as WebSocketWithId;
    simulator['subscriptions'].set(mockKey, [otherWebsocket, mockWebSocket]);

    // Act: Call subscribe with existing WebSocket and message key
    simulator.unsubscribe(mockMessage, mockWebSocket);

    // Assert: Check that the subscription exists with only the other websocket left
    expect(simulator['subscriptions'].size).toBe(1);
    expect(simulator['subscriptions'].get(mockKey)).toEqual([otherWebsocket]);
  });

  test('remove subscription if websocket is the last one', () => {
    // Arrange: Add a subscription with same key and same websocket
    simulator['subscriptions'].set(mockKey, [mockWebSocket]);

    // Act: Call unsubscribe with existing WebSocket and message key
    simulator.unsubscribe(mockMessage, mockWebSocket);

    // Assert: Check that the subscription has been removed
    expect(simulator['subscriptions'].size).toBe(0);
  });

  test('calls sendMessageToClient if unsubscribe is done on non-existing subscription', () => {
    // Arrange: Manually add a subscription with another key
    const key = `otherId-${mockMessage.tree}`;
    simulator['subscriptions'].set(key, [mockWebSocket]);

    // Act: Call unsubscribe with existing WebSocket and non-existing message key
    simulator.unsubscribe(mockMessage, mockWebSocket);

    // Assert: Check that sendMessageToClient was called with the expected error message
    expect(sendMessageToClientMock).toHaveBeenCalledWith(
      mockWebSocket,
      expect.objectContaining({
        category: 'unsubscribe',
        statusCode: 400,
        message: expect.stringContaining('Cannot unsubscribe')
      })
    );
  });

  test('stops the timer if there is no subscription left', () => {
    // Arrange: create a subscription to start the timer
    simulator.subscribe(mockMessage, mockWebSocket);
    expect(simulator['intervalId']).not.toBeNull();

    // Act: Call unsubscribe to trigger the timer removal
    simulator.unsubscribe(mockMessage, mockWebSocket);

    // Assert: Check that intervalId is null
    expect(simulator['intervalId']).toBeNull();
  });

  /*
   * unsubscribeClient
   */

  test('remove only subscriptions of one websocket', () => {
    // Arrange: Add some subscriptions with different websockets
    const otherWebsocket = { id: 'otherWs' } as WebSocketWithId;
    simulator['subscriptions'].set('key-this-and-other', [otherWebsocket, mockWebSocket]);
    simulator['subscriptions'].set('key-only-this', [mockWebSocket]);
    simulator['subscriptions'].set('key-only-other', [otherWebsocket]);

    // Act: Call unsubscribeClient with existing WebSocket
    simulator.unsubscribeClient(mockWebSocket);

    // Assert: Check that websockets were removed from subscriptions
    expect(simulator['subscriptions'].size).toBe(2);
    expect(simulator['subscriptions'].get('key-this-and-other')).toEqual([otherWebsocket]);
    expect(simulator['subscriptions'].get('key-only-this')).toBe(undefined);
    expect(simulator['subscriptions'].get('key-only-other')).toEqual([otherWebsocket]);
  });
  
  test('remove all subscriptions of websocket and none left', () => {
    // Arrange: Add 2 subscriptions with the same websocket
    simulator['subscriptions'].set(mockKey, [mockWebSocket]);
    simulator['subscriptions'].set('other-key', [mockWebSocket]);

    // Act: Call unsubscribeClient with existing WebSocket
    simulator.unsubscribeClient(mockWebSocket);

    // Assert: Check that the subscription has been removed
    expect(simulator['subscriptions'].size).toBe(0);
  });
  
  test('stops the timer if there is no subscription left', () => {
    // Arrange: create a subscription to start the timer
    simulator.subscribe(mockMessage, mockWebSocket);
    expect(simulator['intervalId']).not.toBeNull();

    // Act: Call unsubscribeClient to trigger the timer removal
    simulator.unsubscribeClient(mockWebSocket);

    // Assert: Check that intervalId is no longer null
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

    // Add an entry to `subscriptions` to ensure `checkForChanges` will be called
    const mockWebSocket = { id: 'ws1' } as WebSocketWithId;
    simulator['subscriptions'].set('vehicle123-VSS', [mockWebSocket]);
    
    (jest.spyOn(simulator as any, 'checkForChanges') as jest.Mock)
      .mockImplementation((id: string, upperLimit: number) => {
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

    const { lower: lower1, upper: upper1 } = timeIntervals[0];
    const { lower: lower2, upper: upper2 } = timeIntervals[1];

    // Assertions
    expect(upper1).toBeGreaterThan(lower1); // First interval: upper > lower
    expect(upper2).toBeGreaterThan(lower2); // Second interval: upper > lower
    expect(lower2).toBe(upper1);            // Second interval's lower is the first interval's upper
  });

  test('calls sendMessageToClient if there is a matching subscription and update', async () => {
    simulator["timeIntervalLowerLimit"] = Date.now();
  
    // Add a subscription that should match the `id` in the mocked session data
    simulator['subscriptions'].set('vehicle123-VSS', [mockWebSocket]);
  
    // Mock checkForChanges to return an update message
    jest.spyOn(simulator as any, 'checkForChanges').mockResolvedValueOnce({
      type: "update",
      id: 'vehicle123',
      tree: 'VSS',
      uuid: 'test-uuid',
      nodes: [{ name: 'speed', value: 100 }],
    } as Message); // Return type MessageWithNodes
  
    // Call notifyDatabaseChanges
    await (simulator as any).notifyDatabaseChanges();
  
    // Assert that sendMessageToClient was called with the matching websocket and update message
    expect(sendMessageToClientMock).toHaveBeenCalledWith(
      { id: 'ws1' }, // WebSocket in the subscription
      expect.objectContaining({
        id: 'vehicle123',
        type: 'update',
        nodes: [{ name: 'speed', value: 100 }],
      })
    );
  });
  
});
