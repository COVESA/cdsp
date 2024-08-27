# IoTDB

This directory contains the IoTDB Handler as a Node.js application. As [Apache IoTDB](https://iotdb.apache.org/) is a time-series database, the IoTDB Handler connects to an existing IoTDB instance using the Thrift protocol. The handler utilizes the [IoTDB Thrift API](https://github.com/apache/thrift) to communicate with the database and perform operations. Configuration details, such as the IoTDB host, port, user credentials, and time zone, are specified in the handler's configuration file. The IoTDB Handler is designed to manage sessions, execute queries, and interact with the IoTDB instance efficiently during runtime.

# Features

- **Authentication**: Authenticates with IoTDB using the IoTDB host, port, user credentials, and time zone.
- **Error Handling**: Logs and handles errors during database operations and synchronization.

# Installation

Execute within `iotdb` directory

```bash
npm install
```

# Configure IoTDB

Before the Database-Router can start the IoTDB Handler without any errors you need to start and run Docker containers defined in a [Docker Compose file](/docker/).

## Configure of a IoTDB Handler

Create `config/config.js` with the following format, replacing the app id and the api key with yours.

```js
module.exports = {
  iotdbHost: "your-iotdb-host", // Default "localhost"
  iotdbPort: 6667, // Set this to the appropriate IotDB Port
  iotdbUser: "your-iotdb-user", // Default "root"
  iotdbPassword: "your-iotdb-password", // Default "root"
  timeZoneId: Intl.DateTimeFormat().resolvedOptions().timeZone, // Set this to the appropriate time zone
  fetchSize: 10000, // number of rows that will be fetched from the database at a time when executing a query 
};
```

> **_IMPORTANT:_** Do not commit this file to github!

## Starting the IoTDB handler

You do not need to start IotDB Handler manually. It is started by the DB-Router like described [here](../../../router/README.md#Run).