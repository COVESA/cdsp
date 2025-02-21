# IoTDB

This directory contains the IoTDB Handler as a Node.js application. As [Apache IoTDB](https://iotdb.apache.org/) is a time-series database, the IoTDB Handler connects to an existing IoTDB instance using the Thrift protocol. The handler utilizes the [IoTDB Thrift API](https://github.com/apache/thrift) to communicate with the database and perform operations. Configuration details, such as the IoTDB host, port, user credentials, and time zone, are specified in the handler's configuration file. The IoTDB Handler is designed to manage sessions, execute queries, and interact with the IoTDB instance efficiently during runtime.

# Features

- **Authentication**: Authenticates with IoTDB using the IoTDB host, port, user credentials, number of rows to be fetched, and time zone.
- **Get Data**: Retrieves data from the IoTDB using a VIN as object ID.
- **Set Data**: Write data to the IoTDB using a VIN as object ID.
- **Error Handling**: Logs and handles errors during database operations and synchronization.

# Configure IoTDB

Before the Database-Router can start the IoTDB Handler without any errors you need to start and run Docker containers defined in a [Docker Compose file](/docker/).

## Configure of a IoTDB Handler

Create (if it does not exist) `/docker/.env` and add the following environment variables, replacing the values with yours.

```shell
    #########################
    # GENERAL CONFIGURATION #
    #########################
    
    # HANDLER_TYPE define the database to initialize
    HANDLER_TYPE=iotdb
    # DATA_POINTS_SCHEMA_FILE is the YAML or JSON file containing all data points supported. See the ../../config/README.md for more information.
    DATA_POINTS_SCHEMA_FILE=vss_data_points.yaml
    
    #######################
    # IOTDB CONFIGURATION #
    #######################
    
    # Access to iotdb-service. All these are optional, they have an predefine default value
    IOTDB_HOST="your-iotdb-host" # Docker container name for IoTDB or host, default container name "iotdb-service"
    IOTDB_PORT=6667 # Set this to the appropriate IotDB Port, default "6667"
    IOTDB_USER="your-iotdb-user" # Default "root"
    IOTDB_PASSWORD="your-iotdb-password" # Default "root"
    IOTDB_TIMEZONE="your-time-zone" # Default your local configured time zone
    FETCH_SIZE=10000 #number of rows that will be fetched from the database at a time when executing a query, default 10000
    IOTDB_POLL_INTERVAL_LEN_IN_SEC=5 #number of seconds of interval to poll for changes on the IoTDB
```

> [!WARNING] 
> Do not commit this file to GitHub!

## Starting the IoTDB handler

You do not need to start IotDB Handler manually. It is started by the Websocket-Server like described [here](../../../README.md).
