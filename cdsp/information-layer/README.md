# Information Layer

The information layer is responsible for providing a raw data access [API](#api) (read, write, subscribe).
Its intention is to abstract the underlying data storage database technology.

Clients can interact with it using websockets and JSON payload.


The information-layer consists of two logical components:
- database handler: Interaction with the chosen database, configurable
- router: API provider that connects to the database handler

```mermaid
flowchart LR
    client <--> router
    router <--> handlers
    handlers <--> realm_handler
    handlers <--> iotdb_handler
    realm_handler <--> realmdb
    iotdb_handler <--> iotdb
```

# Hello World Setup

### Build the typescript application natively

Make sure all libraries are installed
```bash
npm install
```
Build the application
```bash
npm run build
```
Run the unit tests
```bash
npx jest
```
or
```bash
npx jest --verbose
```

## Backend Option 1: IoTDB

### Start the database

Start the db:
```bash
docker run -d --rm --name iotdb -p 6667:6667 -p 9003:9003 apache/iotdb:latest
```
Connect to it via cli:
```bash
docker exec -it iotdb ./start-cli.sh -h 127.0.0.1 -p 6667 -u root -pw root
```
Create a database
```bash
create database root.Vehicles
```
Create desired timeseries

```bash
create timeseries root.Vehicles.Vehicle_TraveledDistance WITH DATATYPE=FLOAT, ENCODING=RLE
create timeseries root.Vehicles.Vehicle_Speed WITH DATATYPE=FLOAT, ENCODING=RLE
```

### Start router

Build the router image
```bash
docker build -t router .
```
Run the router
```bash
# Docker
docker run --rm --name router -p 8080:8080 -e HANDLER_TYPE=iotdb -e IOTDB_HOST=host.docker.internal router
# OR natively
npm install
HANDLER_TYPE=iotdb IOTDB_HOST=localhost npm start
```

## Backend Option 2: RealmDB


### Prepare cloud

- Ensure that in your [ATLAS cloud](https://cloud.mongodb.com/) app there is a vehicle _document_ with an
  `Vehicle_VehicleIdentification_VIN` in a collection named _`Vehicles`_.
- Ensure that this document as well contains VSS data. Here you can see the data supported in this repository for a vehicle document within
  _Vehicles_ that should be reflected in ATLAS:

```
_id: "<SOME_STRING>" (String)
Vehicle_Speed: <SOME_DOUBLE> (Double)
Vehicle_TraveledDistance: "<SOME_DOUBLE>" (Double)
```

### Start router

Build the router image
```bash
docker build -t router .
```
Run the router
```bash
# Docker
docker run --rm --name router -p 8080:8080 -e HANDLER_TYPE=realmdb -e VERSION_REALMDB_SCHEMA=0 -e REALM_APP_ID=<YOUR-APP-ID> -e REALM_API_KEY=<YOUR-API-KEY> router
# OR natively
npm install
HANDLER_TYPE=realmdb VERSION_REALMDB_SCHEMA=0 REALM_APP_ID=<YOUR-APP-ID> REALM_API_KEY=<YOUR-API-KEY> npm start
```

## Usage

See [api](#api) how to interact with the router.

# API

Connect your own websocket client by connecting to `ws://localhost:8080`.

The examples use [websocat](https://github.com/vi/websocat) and [jq](https://github.com/jqlang/jq)

## Get

Schema:
```yaml
# only the root node provided to get all data points
{
  "type": "get",
  "instance": "WBY11CF080CH470711_test1",
  "schema": "Vehicle",
  "requestId": "123-456"
}
# with path to non-leaf node to get multiple data points below this node
{
  "type": "get",
  "instance": "WBY11CF080CH470711_test1",
  "schema": "Vehicle",
  "path": "CurrentLocation",
  "requestId": "123-456"
}
# with path to leaf node to get one data point
{
  "type": "get",
  "instance": "WBY11CF080CH470711_test1",
  "schema": "Vehicle",
  "path": "CurrentLocation.Latitude",
  "requestId": "123-456"
  }
```

Example (websocat currently not working because of racing condition between DB connection creation and DB request):
```bash
echo '{"type": "get","instance": "WBY11CF080CH470711_test1","schema": "Vehicle","path": "Speed","requestId": "123-456"}' | websocat ws://localhost:8080 | jq
```

## Set

Schema:
```yaml
# leaf node with single value in data
{
  "type": "set",
  "path": "CurrentLocation.Latitude",
  "instance": "WBY11CF080CH470711_test1",
  "schema": "Vehicle",
  "data": 21,
  "requestId": "235-183"
}
# root node with multiple values in data nested and flat (no path provided)
{
  "type": "set",
  "instance": "WBY11CF080CH470711_test1",
  "schema": "Vehicle",
  "data": {
    "CurrentLocation": {
      "Latitude": 22,
      "Longitude": 46
    },
    "Chassis.SteeringWheel.Angle": 22
  },
  "requestId": "235-183"
}
# non-leaf node with multiple values in data
{
  "type": "set",
  "instance": "WBY11CF080CH470711_test1",
  "schema": "Vehicle",
  "path": "CurrentLocation",
  "data": {
    "Latitude": 22,
    "Longitude": 46
  },
  "requestId": "235-183"
}
```
Example:
```bash
echo '{"type": "set","path": "CurrentLocation.Latitude","instance": "WBY11CF080CH470711_test1","schema": "Vehicle","data": 21}' | websocat ws://localhost:8080 | jq
```
```yaml
{
  "type": "status",
  "code": 200,
  "message": "Successfully wrote data to database.",
  "timestamp": {
    "seconds": 1738613888,
    "nanos": 143000000
  }
}
```

## Subscribe

Schema:
```yaml
{
  "type": "subscribe",
  "instance": "WBY11CF080CH470711_test1",
  "schema": "Vehicle"
}
```

On success:
```yaml
{
  "type": "status",
  "code": 200,
  "message": "Successfully subscribed",
  "timestamp": {
    "seconds": 1738613971,
    "nanos": 955000000
  }
}
```
## Unsubscribe

```yaml
{
  "type": "unsubscribe",
  "schema": "Vehicle",
  "instance": "WBY11CF080CH470711_test1"
}
```

On success:
```yaml
{
  "type": "status",
  "code": 200,
  "message": "Successfully unsubscribed",
  "timestamp": {
    "seconds": 1738614001,
    "nanos": 562000000
  }
}
```

