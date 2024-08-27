This directory contains files related to the [Information Layer](https://en.wikipedia.org/wiki/DIKW_pyramid) of the Central Data Service Playground.

# Information Layer Playground components
   - [Database-Handlers](./handlers)
   - [Database-Router](./router)

# Setting up a CDSP Information Layer
Setting up an information layer in the CDSP involves running a [Database-Router](./router), which is technically a configurable Websocket server that enables northbound connections for WebSocket clients, such as a [Knowledge Layer Connector](../knowledge-layer/README.md), to read, subscribe, and write data. The WebSocket server is connected to a database, which can be selected before starting the web server. Southbound feeders write data to the database in a predefined semantic format, such as [VSS](https://github.com/COVESA/vehicle_signal_specification).  

## Installation of Database-Handler

Please follow installation instructions of the chosen [handler](./handlers/).
 
## Installation of Database-Router
See [hier](./router/README.md#Install) how to install the Database-Router.

# Running "Hello World" example

The Hello World example in our case is quite simple. We feed an updated value for the `CurrentLocation_Longitude` into the database and we check afterwards in the logs if the DB-router creates a Websocket update message for it.

## Choose and prepare your Database

> [!WARNING]
> Before start the application, ensure that the supported endpoints are correctly defined and configured. See [how](./handlers/config/README.md). 

### Realm
- Ensure that in your [ATLAS cloud](https://cloud.mongodb.com/) app there is a vehicle *document* with an `Vehicle_VehicleIdentification_VIN` in a collection named *`Vehicles`*.
- Ensure that this document as well contains VSS data. Here you can see the supported data in a vehicle document within the *Vehicles* should look like in ATLAS:

  ```  
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
- Ensure to start and run Docker containers defined in a [Docker Compose file](/docker/).
- Ensure that in the IoTDB CLI there is a `root.Vehicles` *database* like this: 

  ``` 
  IoTDB> show databases;
  +-------------+----+-----------------------+---------------------+---------------------+
  |     Database| TTL|SchemaReplicationFactor|DataReplicationFactor|TimePartitionInterval|
  +-------------+----+-----------------------+---------------------+---------------------+
  |root.Vehicles|null|                      1|                    1|            604800000|
  +-------------+----+-----------------------+---------------------+---------------------+
  Total line number = 1
  It costs 0.004s
  ```

- Create two *timeseries* with the `root.Vehicles.Vehicle_VehicleIdentification_VIN` and some VSS data. Here you can see an example how the vehicle document within the *Vehicles* should look like in IoTDB CLI:

  ```
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
 
## Start the Database Router

See [here](./router/README.md#Run) how to start the database router.

## Look out for the Websocket Server message in the console
If the handler is running and you are [subscribed](#subscribing-to-changes) to an element, when you change the value of `CurrentLocation_Longitude` in ATLAS cloud (let's say `-157845.68200000003`), you should immediately see this line in console:

```
{
  type: 'update',
  tree: 'VSS',
  id: '<SUBSCRIBED_VIN>',
  dateTime: '<ACTUAL_DATA_TIME>',
  uuid: '<YOUR_SUBSCRIBED_UUID>',
  node: { name: 'CurrentLocation_Longitude', value: `-157845.68200000003` }
}
```

# Connect your own Websocket Client
Connect your own websocket client by connecting to `ws://localhost:8080`

The examples use the VIN (Vehicle Identification Number) as object identifier.

### Reading data

To read data, send a message with the type of request and VIN as object ID:

  * Example for one Node:
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
  * Example for multiple Nodes:
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

  * Example for one Node:
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
  * Example for multiple Nodes:
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
    "uuid": "SOME_UUID"
  }
  ```
    
