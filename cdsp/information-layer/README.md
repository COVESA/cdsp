# Information Layer Playground components

This directory contains files related to the [Information Layer](https://en.wikipedia.org/wiki/DIKW_pyramid) of the Central Data Service Playground.

- [Database-Handlers](./handlers/README.md)
- [Database-Router](./router/README.md)

# Setting up a CDSP Information Layer

Setting up an information layer in the CDSP involves running a [Database-Router](./router/README.md), which is technically a configurable Websocket server that enables northbound connections for WebSocket clients, such as a [Knowledge Layer Connector](../knowledge-layer/README.md), to read, subscribe, and write data. The WebSocket server is connected to a database, which can be selected before starting the web server. Southbound feeders write data to the database in a predefined semantic format, such as [VSS](https://github.com/COVESA/vehicle_signal_specification).

## Installation of Database-Handler

Please follow installation instructions of the chosen [handler](./handlers/README.md).

# Running "Hello World" example (WIP)

The Hello World example in our case is quite simple. We feed an updated value for the `CurrentLocation_Longitude` into the database and we check afterwards in the logs if the DB-router creates a Websocket update message for it.

## Choose and prepare your Database

> [!WARNING]
> Before start the application, ensure that the supported data points are correctly defined and configured. See [how](./handlers/config/README.md).

### Realm

- Ensure that in your [ATLAS cloud](https://cloud.mongodb.com/) app there is a vehicle _document_ with an `Vehicle_VehicleIdentification_VIN` in a collection named _`Vehicles`_.
- Ensure that this document as well contains VSS data. Here you can see the data supported in this repository for a vehicle document within _Vehicles_ that should be reflected in ATLAS:

  ```text
  _id: "<SOME_STRING>" (String)
  Vehicle_Chassis_SteeringWheel_Angle: <SOME_INT> (Int32)
  Vehicle_CurrentLocation_Latitude: <SOME_DOUBLE> (Double)
  Vehicle_CurrentLocation_Longitude: <SOME_DOUBLE> (Double)
  Vehicle_Powertrain_TractionBattery_NominalVoltage: <SOME_INT> (Int32)
  Vehicle_Powertrain_TractionBattery_StateOfCharge_CurrentEnergy: <SOME_DOUBLE> (Double)
  Vehicle_Powertrain_Transmission_CurrentGear: <SOME_INT> (Int32)
  Vehicle_Speed: <SOME_DOUBLE> (Double)
  Vehicle_VehicleIdentification_VIN: "<SOME_STRING>" (String)
  ```

### IoTDB

- Ensure to start and run Docker containers defined in a [Docker Compose file](/docker/README.md).
- Ensure that in the IoTDB CLI there is a `root.Vehicles` _database_ like this:

  ```text
  IoTDB> show databases;
  +-------------+----+-----------------------+---------------------+---------------------+
  |     Database| TTL|SchemaReplicationFactor|DataReplicationFactor|TimePartitionInterval|
  +-------------+----+-----------------------+---------------------+---------------------+
  |root.Vehicles|null|                      1|                    1|            604800000|
  +-------------+----+-----------------------+---------------------+---------------------+
  Total line number = 1
  It costs 0.004s
  ```

- Create two _timeseries_ with the `root.Vehicles.Vehicle_VehicleIdentification_VIN` and some VSS data. Here you can see an example how the vehicle document within the _Vehicles_ should look like in IoTDB CLI:

  ```text
  IoTDB> show timeseries;
  +----------------------------------------------------------------------------+-----+-------------+--------+--------+-----------+----+----------+--------+------------------+--------+
  |                                                                  Timeseries|Alias|     Database|DataType|Encoding|Compression|Tags|Attributes|Deadband|DeadbandParameters|ViewType|
  +----------------------------------------------------------------------------+-----+-------------+--------+--------+-----------+----+----------+--------+------------------+--------+
  |                             root.Vehicles.Vehicle_VehicleIdentification_VIN| null|root.Vehicles|    TEXT|   PLAIN|        LZ4|null|      null|    null|              null|    BASE|
  |                              root.Vehicles.Vehicle_CurrentLocation_Latitude| null|root.Vehicles|  DOUBLE|     RLE|        LZ4|null|      null|    null|              null|    BASE|
  |                             root.Vehicles.Vehicle_CurrentLocation_Longitude| null|root.Vehicles|  DOUBLE|     RLE|        LZ4|null|      null|    null|              null|    BASE|
  |                                                 root.Vehicles.Vehicle_Speed| null|root.Vehicles|   FLOAT|     RLE|        LZ4|null|      null|    null|              null|    BASE|
  |                           root.Vehicles.Vehicle_Chassis_SteeringWheel_Angle| null|root.Vehicles|   INT32|     RLE|        LZ4|null|      null|    null|              null|    BASE|
  |                   root.Vehicles.Vehicle_Powertrain_Transmission_CurrentGear| null|root.Vehicles|   INT32|     RLE|        LZ4|null|      null|    null|              null|    BASE|
  |root.Vehicles.Vehicle_Powertrain_TractionBattery_StateOfCharge_CurrentEnergy| null|root.Vehicles|   FLOAT|     RLE|        LZ4|null|      null|    null|              null|    BASE|
  |             root.Vehicles.Vehicle_Powertrain_TractionBattery_NominalVoltage| null|root.Vehicles|   INT32|     RLE|        LZ4|null|      null|    null|              null|    BASE|
  +----------------------------------------------------------------------------+-----+-------------+--------+--------+-----------+----+----------+--------+------------------+--------+
  ```

## Start the Database Router

### Using Docker Compose

See [here](/docker/README.md) how to start the services using Docker.

### Using Docker directly from Dockerfile in information-layer

Go to `covesa.cdsp/cdsp/information-layer` and run in CLI:

```shell
$ docker build -t webserver-service .
```

To run `RealmDB` use:

> [!IMPORTANT] See [here](./handlers/src/realmdb/README.md) all the required ENV variables.

```shell
$ docker run --name websocket-service -e HANDLER_TYPE=realmdb [-e required_env_variables] -p 8080:8080 websocket-service
```

To run `IoTDB` use:

> [!IMPORTANT] See [here](./handlers/src/iotdb/README.md) all the required ENV variables. These are optional, because by default it uses the local configuration to connect to the `iotdb-service` container.

```shell
$ docker run --name websocket-service -e HANDLER_TYPE=iotdb --network cdsp_default [-e required_env_variables] -p 8080:8080 websocket-service
```

### Using Node.js

#### Install

Execute in this directory:

```shell
npm install
```

To start the websocket-server using `RealmDB` execute the command:

To run `RealmDB` use:

> [!IMPORTANT] See [here](./handlers/src/realmdb/README.md) all the required ENV variables.
 
```shell
$ HANDLER_TYPE=realm [ENV_VARIABLE_NAME required_env_variables] npm start
```

To start the websocket-server using `IoTDB` execute the command:

> [!IMPORTANT] See [here](./handlers/src/iotdb/README.md) all the required ENV variables. These are optional, because by default it uses the local configuration to connect to the `iotdb-service` container.

```shell
$ HANDLER_TYPE=iotdb IOTDB_HOST=localhost [ENV_VARIABLE_NAME required_env_variables] npm start 
```

## Look out for the Websocket Server message in the console

If the handler is running, and you are [subscribed](#subscribing-to-changes) to an element, when you change the value of `CurrentLocation_Longitude` in ATLAS cloud (let's say `-157845.68200000003`), you should immediately see this line in console:

```json
{
  "type": "update",
  "tree": "VSS",
  "id": "<SUBSCRIBED_VIN>",
  "dateTime": "<ACTUAL_DATA_TIME>",
  "uuid": "<YOUR_SUBSCRIBED_UUID>",
  "node": { "name": "CurrentLocation_Longitude", "value": "-157845.68200000003" }
}
```

If the handler response with some error during any request, the client may get error response similar to this (with the same format):

```json
{
  "type": "<MESSAGE_TYPE>:status",
  "errorCode": 404,
  "error": "<ERROR_DESCRIPTION>"
}
```

# Connect your own Websocket Client

Connect your own websocket client by connecting to `ws://localhost:8080`

The examples use the VIN (Vehicle Identification Number) as object identifier.

### Reading data

To read data, send a message with the type of request and VIN as object ID:

- Example for one Node:
  ```json
  {
    "type": "read",
    "tree": "VSS",
    "id": "<SOME_VIN>",
    "uuid": "<SOME_UUID>",
    "node": {
      "name": "<SOME_ENDPOINT>"
    }
  }
  ```
- Example for multiple Nodes:
  ```json
  {
    "type": "read",
    "tree": "VSS",
    "id": "<SOME_VIN>",
    "uuid": "<SOME_UUID>",
    "nodes": [
      {
        "name": "<SOME_ENDPOINT>"
      },
      {
        "name": "<OTHER_ENDPOINT>"
      }
    ]
  }
  ```

### Writing data

To write data, send a message with the type of request and VIN as object ID (at this moment only with IoTDB available):

- Example for one Node:
  ```json
  {
    "type": "write",
    "tree": "VSS",
    "id": "<SOME_VIN>",
    "uuid": "<SOME_UUID>",
    "node": {
      "name": "<SOME_ENDPOINT>",
      "value": "<some_value>"
    }
  }
  ```
- Example for multiple Nodes:
  ```json
  {
    "type": "write",
    "tree": "VSS",
    "id": "<SOME_VIN>",
    "uuid": "<SOME_UUID>",
    "nodes": [
      {
        "name": "<SOME_ENDPOINT>",
        "value": "<some_value>"
      },
      {
        "name": "<OTHER_ENDPOINT>",
        "value": "<other_value>"
      }
    ]
  }
  ```

### Subscribing to changes

To subscribe to changes in a specific object, send a message with the type of request and VIN as object ID (at this moment only with RealmDB available):

```json
{
  "type": "subscribe",
  "tree": "VSS",
  "id": "<SOME_VIN>",
  "uuid": "<SOME_UUID>"
}
```

If the subscription succeed, the server will respond with the following message:

```json
{
  "type": "subscribe:status",
  "tree": "VSS",
  "id": "<SOME_VIN>",
  "dateTime": "2024-09-12T15:50:17.232Z",
  "uuid": "<SOME_UUID>",
  "status": "succeed"
}
```
### Unsubscribing to changes

To unsubscribe to changes in a specific object, send a message with the type of request and VIN as object ID (at this moment only with RealmDB available):

```json
{
  "type": "unsubscribe",
  "tree": "VSS",
  "id": "<SOME_VIN>",
  "uuid": "<SOME_UUID>"
}
```
If the unsubscription succeed, the server will respond with the following message:
```json
{
  "type": "unsubscribe:status",
  "tree": "VSS",
  "id": "WBY11CF080CH470711",
  "dateTime": "2024-09-12T17:40:00.754Z",
  "uuid": "CLIENT_1_SUBS",
  "status": "succeed"
}
```

