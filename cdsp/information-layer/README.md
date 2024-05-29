This directory contains files related to the [Information Layer](https://en.wikipedia.org/wiki/DIKW_pyramid) of the Central Data Service Playground.

## 1. Information Layer Playground components
- Database-Handlers
    - [RealmDB-Handler](realmdb/README.md)
    - [IotDB-Handler](iotdb/README.md)
- [Database-Router](db-router/README.md)


## 2. Setting up a CDSP Information Layer
Setting up an information layer in the CDSP involves running a [Database-Router](db-router/README.md), which is technically a configurable Websocket server that enables northbound connections for WebSocket clients, such as a [Knowledge Layer Connector](../knowledge-layer/README.md), to read, subscribe, and write data. The WebSocket server is connected to a database, which can be selected before starting the web server. Southbound feeders write data to the database in a predefined semantic format, such as [VSS](https://github.com/COVESA/vehicle_signal_specification).  

### 2.1 Installation of Database-Handler

Install and configure your database of choice, like described here for [RealmDB](realmdb/README.md) or for [IotDB](iotdb/README.md)
    
### 2.2 Installation of Database-Router

- Install and configure [Database-Router](db-router/README.md)

## 3. Running "Hello World" example
The Hello World example in our case is quite simple. We feed an updated value for the HVAC ambient air temperature into the database and we check afterwards in the logs if the DB-router creates a Websocket update message for it.
### 3.1  Choose and prepare your Data Feedeer

<b>Case 1: You choosed Realm as Database:</b>

- Ensure that in your [ATLAS cloud](https://cloud.mongodb.com/) app there is a vehicle <i>document</i> with an <code>_id: <i>1234567</i></code> in a collection named <i>Vehicles</i>. 
- Ensure that this document as well contains VSS data. At the moment only 1 data point is supported, namely Vehicle.Cabin.HVAC.AmbientAirTemperature. Here you can see how the vehicle document within the<i>Vehicles</i> should like in ATLAS:
<code><br>
_id: 1234567 (Int64) <br>
Vehicle_Cabin_HVAC_AmbientAirTemperature: 6 (Double) <br>
</code>

<b>Case 2: You choosed IoTDB as Database:</b>

- Not yet supported

### 3.2 Start the Database Router
Start DB-Router by executing in <code>db-router/src</code> directory the command

    
     node websocket-server.js
    

### 3.3 Look out for the Websocket Server message in the console
Now you can changed the value of <i>Vehicle_Cabin_HVAC_AmbientAirTemperature</i> in ATLAS cloud to let's say 23. After changing you should immediately see this line in console

    * the value of "Vehicle_Cabin_HVAC_AmbientAirTemperature" changed to 23


### 3.4 Connect your own Websocket Client
Connect your own websocket client by connecting to <i>ws://localhost:8080</i>