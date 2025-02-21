# Database handlers

This project already contains handlers configured to be used with RealmDB and IoTDB.

## Adding a New Database Handler

This project uses a handler interface to dynamically integrate new database backends such as RealmDB or IoTDB. Each handler must implement the core functionality to handle WebSocket messages (get, set, subscribe, unsubscribe).

### How to Add a New Database Handler

1. **Create a new handler class**: 
    Create a new file for your database handler (e.g., `mydb-handler.ts`) in the `./mydb/src` directory. This handler should extend the base class from [HandlerBase.js](./HandlerBase.ts).
2. **Implement the handler methods**: 
    You must implement the following methods in your new handler:
   - `authenticateAndConnect()`: Establish a connection with the database and authenticate.
   - `get(message, ws)`: Retrieve data from the database based on the incoming WebSocket message.
   - `set(message, ws)`: Write data to the database.
   - `subscribe(message, ws)`: Subscribe to changes in the database, and automatically send updates over WebSocket.
   - `unsubscribe(message, ws)`: Unsubscribe from database updates.

3. **Example Handler Implementation**:
   Here’s a basic template you can follow:
```js
const Handler = require('../../handler');

class MyDBHandler extends Handler {
async authenticateAndConnect() {
    // Connect to your database here
}

async get(message, ws) {
    // Implement the logic to read data from the database
}

async set(message, ws) {
    // Implement the logic to write data to the database
}

async subscribe(message, ws) {
    // Implement the logic to subscribe to updates from the database
}

async unsubscribe(message, ws) {
    // Implement the logic to unsubscribe from updates
}
}

module.exports = MyDBHandler;
```

4. **Create configuration files**: 
    Create the the configuration files into `./mydb/config` to include parameters for your new database (e.g., database names, data schemas, etc.).
> [!IMPORTANT]     
> Ensure to create the necessary files to support the necessary data points that will be store in the DB and required for your clients. See [how](../config/README.md).

5. **Work with the Handler**: 

    Create (if it does not exist) `/docker/.env` and add the following environment variables, replacing the values with yours:

```sh
#########################
# GENERAL CONFIGURATION #
#########################

# HANDLER_TYPE define the database to initialize
HANDLER_TYPE=mydb
# DATA_POINTS_SCHEMA_FILE is the YAML or JSON file containing all data points supported. See the ../../config/README.md for more information.
DATA_POINTS_SCHEMA_FILE=vss_data_points.yaml
#########################
# MYDB CONFIGURATION #
#########################

# Other variables are optional, they will not be committed. You can define custom variables like API Keys or secrets.
OPTIONAL_CUSTOM_VARIABLES="value"
```

> [!WARNING] 
> Do not commit this file to GitHub!

In order to work with your custom database handler, it is required to create it in the [HandlerCreator.ts](./HandlerCreator.ts). 

```ts
switch (handlerType) {
  case "realmdb":
    handler = new RealmDBHandler();
    break;
  case "iotdb":
    handler = new IoTDBHandler();
    break;
  // define the new MyDBhandler object.
  case "mydb":
    handler = new MyDBHandler();
    break;
  default:
    throw new Error("Unsupported handler type");
}
```

Run the WebSocket server, and connect with your handler by sending WebSocket messages to test reading, writing, and subscribing functionalities. The handler should be started by the DB-Router like described [here](../../README.md).

### Existing Handlers

You can check the following examples to understand how to structure your new handler:
- **RealmDB Handler**: [RealmHandler](./realmdb/src/RealmDBHandler.ts) provides an example of how to interact with RealmDB.
- **IoTDB Handler**: A similar implementation can be followed for [IoTDBHandler](./iotdb/src/IoTDBHandler.ts).

For additional logging, you can utilize the `logMessage` function from [logger.ts](../../utils/logger.ts).