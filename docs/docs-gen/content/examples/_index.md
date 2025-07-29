---
title: "Playground examples"
weight: 30
---

The following examples are intended to show how to use playground components and how they can be combined to build larger systems.

They are grouped by categories to help discovery of appropriate examples from different viewpoints.

Note: There is not a separate section for the VSS data model because the vast majority of the examples will be making use of it.

## Getting started

| Name | Relationship to the category |
|------|-------------|
| [Docker sanity test](https://github.com/COVESA/cdsp/tree/main/docker#deploy-with-docker-compose) | Simple sanity test for the Playground Docker deployment |
| [Hello-world](https://github.com/COVESA/cdsp/tree/main/examples/cdsp-hello-world) | Simple "hello-world" example|

## Data Layer, Processing and Analysis
Topic examples: Data Reduction, Data Quality, Events, Data Streams etc.

| Name | Relationship to the category |
|------|-------------|
| [vehicle-speed-downsample-iotdb](https://github.com/COVESA/cdsp/tree/main/examples/vehicle-speed-downsample-iotdb) | Accurately down-sample a timeseries of pre-recorded high frequency VSS `Vehicle.Speed` data using the IoTDB Data Quality Library|

Tip: well you wait for more examples consider how you could use the [IoTDB data processing functions]({{< ref "apache-iotdb#data-processing-functions" >}} "IoTDB data processing").

## Knowledge Layer, Reasoning and Data Models
Topic examples: Knowledge Layer Connector, Data Layer Connector etc.

## Feeders
Topic examples: Virtual signal platforms, VISSR etc.

| Name | Relationship to the category |
|------|-------------|
| [RemotiveLabs feeder](https://github.com/COVESA/cdsp/tree/main/examples/remotivelabs-feeder) | Example bridge that streams vehicle data from the RemotiveLabs cloud platform into the IoTDB data store |
| [KUKSA CAN Provider feeder](https://github.com/COVESA/cdsp/tree/main/examples/kuksa-can-feeder) | Using the KUKSA CAN Provider as a feeder of CAN data translated to the VSS data model into the IoTDB data store |

## COVESA Touchpoints
Topic examples: Low level vehicle abstraction, Mobile devices, Car2Cloud / Cloud etc. 

## COVESA Technologies
Topic examples: vsome/ip (SOME/IP), uServices, Vehicle API, VISS etc.

| Name | Relationship to the category |
|------|-------------|
| [VISSR/VISS hello-world](https://covesa.github.io/vissr/build-system/hello-world/) | hello-world tutorial for making VISS requests using VISSR in the upstream VISSR project |
| [VISS transport examples](https://raw.githack.com/COVESA/vehicle-information-service-specification/main/spec/VISSv3.0_Transport.html) | Examples in the VISS Specification for making VISS requests and the responses for various transport protocols |

## Databases
Topic examples: Apache IoTDB, MongoDB Realm, Redis/SQLite/memcache etc.

| Name | Relationship to the category |
|------|-------------|
| [vehicle-speed-downsample-iotdb](https://github.com/COVESA/cdsp/tree/main/examples/vehicle-speed-downsample-iotdb) | Using the IoTDB [Data Quality Library ]({{< ref "apache-iotdb#data-processing-functions" >}} "IoTDB Data Quality Library") for advanced (VSS) timeseries data processing|
| [RemotiveLabs feeder](https://github.com/COVESA/cdsp/tree/main/examples/remotivelabs-feeder) | Example of streaming (writing) southbound (VSS) timeseries data into IoTDB |

## Frameworks / Protocols
Topic examples: vsome/ip (SOME/IP), VISS, uServices/uProtocol/Capabilities, Vehicle API, MQTT, Kafka, Apache Zeppelin etc.

| Name | Relationship to the category |
|------|-------------|
| VISS | Find examples in the COVESA Technologies section and or search for VISS/VISSR in this page |

## Big data
Topic examples: Hadoop, Flink, Spark, Cloud DB,  Nifi etc.

## Other examples
Topics that do not fit into the groups above.