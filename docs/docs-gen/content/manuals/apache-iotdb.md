---
title: "Apache IoTDB"
---

## Introduction
The playground uses Apache IoTDB to provide a highly functional data store.

Description of Apache IoTDB from https://iotdb.apache.org/:

*"Apache IoTDB (Database for Internet of Things) is an IoT native database with high performance for data management and analysis, deployable on the edge and the cloud. Due to its light-weight architecture, high performance and rich feature set together with its deep integration with Apache Hadoop, Spark and Flink, Apache IoTDB can meet the requirements of massive data storage, high-speed data ingestion and complex data analysis in the IoT industrial fields."*

The IoTDB project [website](https://iotdb.apache.org/) has extensive documentation on the IoTDB server. In the guide below we focus on topics specific to the playground such as VSS data schemas and the connector for the VISSR VISS data server.

Info: The intention is to add more information such as a guide for feeder integration as the project progresses.

## Integrating VSS data into the IoTDB data model
The ["Basic Concept"](https://iotdb.apache.org/UserGuide/latest/Basic-Concept/Data-Model-and-Terminology.html) section of the IoTDB documentation introduces the IoTDB data model, data types, encoding and compression.

In IoTDB terminology *measurement* is the key in a key/value pair. In VSS terms the leaf node name. The timeseries is the record of the measurement on the time axis. A timeseries is a series of time/value data points.

The IoTDB data model supports hierarchical partitioning and like VSS uses a dot notation to separate the levels. This means if we simply appended a VSS leaf node name like `Vehicle.CurrentLocation.Longitude` as the measurement (key) name to the end of a IoTDB path such as `root.test2.dev1` the `Vehicle.CurrentLocation.` IoTDB would treat it as part of the IoTDB data model partitioning which could cause unwanted issues when scaling over millions of vehicles.

We have separated those two concepts by quoting the VSS leaf node name using backticks when processing the name in IoTDB. As shown below:
```
<IoTDB prefix path>.`<VSS leaf node name>`
```
In our example the full IoTDB measurement path would become ``root.test2.dev1.`Vehicle.CurrentLocation.Longitude` `` and in an IoTDB timeseries would look like this:
```
+------------------------+---------------------------------------------------+
|                    Time|root.test2.dev1.`Vehicle.CurrentLocation.Longitude`|
+------------------------+---------------------------------------------------+
|2024-03-07T17:55:24.514Z|                                           -42.4567|
|2024-04-10T17:48:12.117Z|                                           -41.3567|
|2024-04-10T17:48:23.389Z|                                           -39.3567|
|2024-04-10T17:48:49.630Z|                                           -40.2578|
+------------------------+---------------------------------------------------+

```

## Seeding the database with VSS data
To illustrate the concepts lets seed a database with some simple timeseries VSS data.

The typical steps are:
1. Create the database in the server.
2. Create a timeseries in the database populated with the VSS leaf nodes (keys) you are interested in.
3. Load the VSS data into the timeseries.

Note: IoTDB has data type detection so creating a schema for the timeseries in step 2 is optional. However, the use of a schema has performance and meta-data benefits so is recommended.

IoTDB has a very extensive collection of integrations, tools, clients and APIs that could be used to achieve this.

### Example using the IoTDB CLI client
The following tutorial shows an example using the [IoTDB CLI client](https://iotdb.apache.org/UserGuide/latest/Tools-System/CLI.html), using two methods. Firstly, in interactive mode where you type the commands and then sending the same commands in batch command mode.


1. Connect to the CLI client from your host:
```
$ bash <iotdb path>/sbin/start-cli.sh -h <server hostname/ip>
```
2. Create database from CLI command line:
```
IoTDB > create database root.test2
```
3. Create timeseries from CLI command line:
```
IoTDB > CREATE ALIGNED TIMESERIES root.test2.dev1(`Vehicle.CurrentLocation.Longitude` FLOAT, `Vehicle.CurrentLocation.Latitude` FLOAT, `Vehicle.Cabin.Infotainment.HMI.DistanceUnit` TEXT)
```
4. Add some data into the timeseries:
```
IoTDB> insert into root.test2.dev1(`Vehicle.CurrentLocation.Longitude`, `Vehicle.CurrentLocation.Latitude`, `Vehicle.Cabin.Infotainment.HMI.DistanceUnit`) values(-42.4567, 22.1234, 'MILES')
```
5. Display the data just added as a sanity check:
```
IoTDB> select last * from root.test2.dev1
+------------------------+-------------------------------------------------------------+--------+--------+
|                    Time|                                                   Timeseries|   Value|DataType|
+------------------------+-------------------------------------------------------------+--------+--------+
|2024-03-07T17:55:24.514Z|          root.test2.dev1.`Vehicle.CurrentLocation.Longitude`|-42.4567|   FLOAT|
|2024-03-07T17:55:24.514Z|root.test2.dev1.`Vehicle.Cabin.Infotainment.HMI.DistanceUnit`|   MILES|    TEXT|
|2024-03-07T17:55:24.514Z|           root.test2.dev1.`Vehicle.CurrentLocation.Latitude`| 22.1234|   FLOAT|
+------------------------+-------------------------------------------------------------+--------+--------+
```
You have now seeded the database with some initial VSS data.

The CLI client startup script accepts SQL commands using the `-e` parameter. We can therefore use this to codify the above in a bash script. So the VSS node names (keys) are passed correctly on the command line the backticks must be escaped.

For example:
```
# !/bin/bash

host=127.0.0.1
rpcPort=6667
user=root
pass=root

bash ./sbin/start-cli.sh -h ${host} -p ${rpcPort} -u ${user} -pw ${pass} -e "create database root.test2"

bash ./sbin/start-cli.sh -h ${host} -p ${rpcPort} -u ${user} -pw ${pass} -e "CREATE ALIGNED TIMESERIES root.test2.dev1(\`Vehicle.CurrentLocation.Longitude\` FLOAT, \`Vehicle.CurrentLocation.Latitude\` FLOAT, \`Vehicle.Cabin.Infotainment.HMI.DistanceUnit\` TEXT)"

bash ./sbin/start-cli.sh -h ${host} -p ${rpcPort} -u ${user} -pw ${pass} -e "insert into root.test2.dev1(\`Vehicle.CurrentLocation.Longitude\`, \`Vehicle.CurrentLocation.Latitude\`, \`Vehicle.Cabin.Infotainment.HMI.DistanceUnit\`) values(-42.4567, 22.1234, 'MILES')"

bash ./sbin/start-cli.sh -h ${host} -p ${rpcPort} -u ${user} -pw ${pass} -e "select last * from root.test2.dev1"
```

Of course any of the programming language clients provided by IoTDB, e.g. go, python, C++, Rust, or integration into tools that support its SQL language, can also be used to achieve the same result.

## Single node (Edge) vs Cluster
The upstream IoTDB project has both standalone single node and cluster deployments, to cover use case requirements from edge to cloud.

At the time of writing the playground docker deployment deploys the single node IoTDB docker image, as it is most suitable for deployment at the edge in-vehicle, whilst still being able to represent the cloud.

Of course if your cloud development requires higher performance then you can integrate the cluster version.

## UDF and UDF library for data processing
Whilst IoTDB has a series of built-in timeseries processing functions you can add your own as User Defined Functions (UDF).

The [UDF section](https://iotdb.apache.org/UserGuide/latest/User-Manual/Database-Programming.html#user-defined-function-udf) of the IoTDB documentation explains how to develop and register your own.

The IoTDB project also maintains UDF Library an extensive collection of data processing functions covering:
- Data Quality
- Data Profiling
- Anomaly Detection
- Frequency Domain Analysis
- Data Repair
- Series Discovery
- Machine Learning

The UDF Library is an optional install. How to install the library is documented [here](https://iotdb.apache.org/UserGuide/latest/User-Manual/Database-Programming.html#data-quality-function-library). Documentation for the functions can be found [here](https://iotdb.apache.org/UserGuide/latest/Reference/UDF-Libraries.html).

The combination of built-in and UDF library functions, built on the low latency queries enabled by IoTDB and its TsFile format gives you a lot to explore.

## VISSR (VISS) integration
As part of the initial development of the playground the team extended VISSR to support connections to Apache IoTDB as a VISSR data store backend and upstreamed the support.

### Connector scope

The support is implemented by connector code in the VISSR service manager, which connects VISSR to an external Apache IoTDB server. This code uses the IoTDB Go client to maintain a connection session to the IoTDB server, which it then uses to get/set vehicle data from the database.

As VISSR and the IoTDB server are separate processes VISSR needs to be told where to find the IoTDB server and which storage prefix to use to access the data.

Development followed the patterns set by the existing VISSR support for Redis and SQLite. The administration of the Apache IoTDB server itself, including startup and shutdown, is out of scope of the connector and is handled externally to VISSR. In the case of the playground this is handled by the playground compose.

### Runtime notes

VISSR runtime assumptions:
1. IoTDB server lifecycle (e.g. startup and shutdown) is handled externally to VISSR.
2. Management (e.g. creation/deletion) of the IoTDB timeseries containing VSS data is handled externally to VISSR.
3. Configuration of the connector code is specified in the config file iotdb-config.json. If the config file is not found then build-time defaults are used.

Handling of IoTDB server and timeseries management is placed outside of VISSR to allow flexible deployment through loosely coupled connections.

### Database schema assumptions
The connector assumes a simple key/value pair schema for accessing VSS data in an IoTDB timeseries:

1. VSS node names (keys) are backtick quoted when stored as measurement keys in the database e.g. `` `Vehicle.CurrentLocation.Longitude` ``. This is for reasons explained above in ["Integrating VSS data into the IoTDB data model"](#integrating-vss-data-into-the-iotdb-data-model) to avoid IoTDB interpreting the VSS tree path, in the example `Vehicle.CurrentLocation.`, as part of its storage path which also uses a dot notation.

2. VSS data is stored using native (IoTDB) data types rather than strings.

3. That the timeseries containing VSS nodes can be found using the prefix path specified in the config file.

### Configuration
The connection code reads its runtime configuration from the JSON formatted file `iotdb-config.json` located in the vissv2server directory. You must specify all values.

#### Configuration file format

| Key name | Type | Description |
| --- | --- | --- 
|`host`|String|Hostname or IP address of the IoTDB server|
|`port`|String|RPC port of the server. Default is 6667|
|`username`|String|Username to access the server. Default is root|
|`password`|String|Password to access the server. Default is root|
|`queryPrefixPath`|String|Timeseries prefix path of VSS data in the database|
|`queryTimeout(ms)`|Int| Query timeout in milliseconds|


Example `iotdb-config.json`:
```
{
	"host": "iotdb-service",
	"port": "6667",
	"username": "root",
	"password": "root",
	"queryPrefixPath": "root.test2.dev1",
	"queryTimeout(ms)": 5000
}
```
### Logging
The connector writes information, warning and error messages to the VISSR server log with the prefix ``IoTDB``. Grepping in the log for that prefix string can help you quickly identify connector messages.

### VISSR Development notes
Please see the notes in the VISSR source commit messages and related Github pull requests for the history of the development of the Apache IoTDB connection code and its integration into the VISSR Service Manager component.

Development followed the patterns set by the existing support for Redis and SQLite.

The connection code was first developed with Apache IoTDB v1.2.2, using the upstream standalone pre-built image and Apache IoTDB Go Client v1.1.7.