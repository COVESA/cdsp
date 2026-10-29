# Database handlers

This project contains one handler configured to be used with IoTDB.

## Adding a New Database Handler

This project uses a handler interface to dynamically integrate new database backends such as IoTDB, RealmDB, and other DB solutions. Each handler must implement the necessary methods and core
functionality to handle WebSocket messages (`get`, `set`, `subscribe`, `unsubscribe`).

### How to Add a New Database Handler

1. **Create a new handler class**
   Create a new file for your database handler (e.g., `mydb-handler.ts`) in the `./mydb/src` directory. This handler should extend the base class from [HandlerBase.ts](./HandlerBase.ts).

2. **Implement the handler methods**
   You must implement the following methods in your new handler:
   - `authenticateAndConnect()`: Establish a connection with the database and authenticate.
   - `getKnownDatapointsByPrefix(prefix)`: Return all known datapoint names that begin with the specified prefix.
   - `getDataPointsFromDB(dataPoints, vin)`: Return datapoints from DB based on requested datapoints and VIN.
   - `set(message, ws)`: Write data to the database.
   - `subscribe(message, ws)`: Subscribe to database changes and send updates over WebSocket.
   - `unsubscribe(message, ws)`: Unsubscribe from database updates.
   - `unsubscribeClient(ws)`: Unsubscribe/cleanup client resources on closed connection.

3. **Example handler implementation**

```ts
import { HandlerBase } from "../HandlerBase";
import { WebSocketWithId } from "../../utils/database-params";
import {
  GetMessageType,
  SetMessageType,
  SubscribeMessageType,
  UnsubscribeMessageType,
} from "../../router/utils/NewMessage";

export class MyDBHandler extends HandlerBase {
  async authenticateAndConnect() {
    // Connect to your database here
  }

  getKnownDatapointsByPrefix(datapointPrefix: string): string[] {
    // Return a list of known datapoints by prefix
    return [];
  }

  async getDataPointsFromDB(dataPoints: string[], vin: string) {
    // Return datapoints + metadata in QueryResult shape
    return { success: true, dataPoints: [], metadata: [] };
  }

  protected async get(
    message: GetMessageType,
    ws: WebSocketWithId,
  ): Promise<void> {
    // Implement get flow
  }

  protected async set(
    message: SetMessageType,
    ws: WebSocketWithId,
  ): Promise<void> {
    // Implement set flow
  }

  protected async subscribe(
    message: SubscribeMessageType,
    ws: WebSocketWithId,
  ): Promise<void> {
    // Implement subscribe flow
  }

  protected async unsubscribe(
    message: UnsubscribeMessageType,
    ws: WebSocketWithId,
  ): Promise<void> {
    // Implement unsubscribe flow
  }
}
```

4. **Create configuration files**
   Create configuration files under ./mydb/config to include parameters for your new database (e.g., database names, data schemas, etc.).

   > [!IMPORTANT]
   > Ensure you create the necessary files to support all datapoints required by your clients. See how.

5. **Work with the handler**:

Create `docker/.env` from the template if it does not exist (`cp docker/.env.example docker/.env`), then set `HANDLER_TYPE` to your handler and add its variables, for example:

```sh
#########################
# GENERAL CONFIGURATION #
#########################

HANDLER_TYPE=mydb

#########################
# MYDB CONFIGURATION    #
#########################

OPTIONAL_CUSTOM_VARIABLES="value"
```

Add the new variables with example values (no secrets) to `docker/.env.example` as well, so the template stays the reference for everyone.

> [!WARNING]
> Do not commit `docker/.env`; it is not tracked in git.

To use your custom handler, register it in [HandlerCreator.ts](./HandlerCreator.ts):

```ts
switch (handlerType) {
  case "iotdb":
    handler = new IoTDBHandler();
    break;
  // define the new MyDBHandler object.
  case "mydb":
    handler = new MyDBHandler();
    break;
  default:
    throw new Error("Unsupported handler type");
}
```

Run the WebSocket server and test read/write/subscribe flows. The handler is started by the DB-Router as described here.

### Existing handlers

- **IoTDB Handler**: example implementation for IoTDB. [IoTDBHandler](./iotdb/src/IoTDBHandler.ts).

### IoTDB note (current state)

The IoTDB runtime path uses the Node.js native client (@iotdb/client) and shared session lifecycle. Legacy Thrift runtime code path has been removed.

For additional logging, use `logMessage` from [logger.ts](../../utils/logger.ts).
