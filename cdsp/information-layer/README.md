This directory contains files related to the [Information Layer](https://en.wikipedia.org/wiki/DIKW_pyramid) of the Central Data Service Playground.

# Information Layer Playground components
   - Database-Handlers
       - [RealmDB-Handler](realmdb/README.md)
       - [IotDB-Handler](iotdb/README.md)
   - [Database-Router](db-router/README.md)

# Setting up a CDSP Information Layer
Setting up an information layer in the CDSP involves running a [Database-Router](db-router/README.md), which is technically a configurable Websocket server that enables northbound connections for WebSocket clients, such as a [Knowledge Layer Connector](../knowledge-layer/README.md), to read, subscribe, and write data. The WebSocket server is connected to a database, which can be selected before starting the web server. Southbound feeders write data to the database in a predefined semantic format, such as [VSS](https://github.com/COVESA/vehicle_signal_specification).  

## Installation of Database-Handler
Install and configure your database of choice, like described here for [RealmDB](realmdb/README.md) or for [IotDB](iotdb/README.md)
 
## [Installation of Database-Router](./db-router/README.md)

# Running "Hello World" example

The Hello World example in our case is quite simple. We feed an updated value for the HVAC ambient air temperature into the database and we check afterwards in the logs if the DB-router creates a Websocket update message for it.

## Choose and prepare your Database

### Realm
- Ensure that in your [ATLAS cloud](https://cloud.mongodb.com/) app there is a vehicle *document* with an `_id: 1234567` in a collection named *Vehicles*.
- Ensure that this document as well contains VSS data. At the moment only 1 data point is supported, namely `Vehicle.Cabin.HVAC.AmbientAirTemperature`. Here you can see how the vehicle document within the *Vehicles* should look like in ATLAS:

  ```
  _id: 1234567 (Int64)
  Vehicle_Cabin_HVAC_AmbientAirTemperature: 6 (Double)
  ```

### IoTDB
- Not yet supported

## Start the Database Router
Start DB-Router by executing in `db-router/src` directory the command

```bash
node websocket-server.js
```

## Look out for the Websocket Server message in the console
Now you can changed the value of *Vehicle_Cabin_HVAC_AmbientAirTemperature* in ATLAS cloud to let's say `23`. After changing you should immediately see this line in console:

```
the value of "Vehicle_Cabin_HVAC_AmbientAirTemperature" changed to 23
```

## Connect your own Websocket Client
Connect your own websocket client by connecting to `ws://localhost:8080`
