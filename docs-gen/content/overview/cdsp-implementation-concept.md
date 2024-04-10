---
title: "Playground implementation"
weight: 30
---


The prior section describing the logical concept should be read to understand the concepts to be implemented. 

As outlined in the logical description, as a starting point the Service is realized as a basic building block combining VISS Data Server with highly functional (VSS) Data Store.

### VISS data server
The reference implementation for the COVESA VISS specification is developed in the [VISSR](https://github.com/COVESA/vissr) project.
The playground has included VISSR to provide its northbound VISS support.

Alongside the VISS server the VISSR project provides clients, feeders and tooling. Further details can be found in the upstream [VISSR documentation site](https://covesa.github.io/vissr/).

During initial development of the playground the upstream VISSR project supported using SQLite, Redis or memcached databases as its data store.

### Highly functional (VSS) data store
As explained earlier in the [logical concept]({{< ref "cdsp-logical-concept.md/#components" >}} "data store logical description") the playground includes the use of highly functional VSS data stores to expand the available data processing possibilities. Two archetypes were considered:
1. Database server with timeseries capabilities
2. Application database

The OSS Apache IoTDB project was selected for the database server archetype for reasons explained in the [section](#apache-iotdb) below.

To enable access to VSS data stored in IoTDB using the VISS protocol the playground project extended VISSR to support IoTDB as one of its supported data store backends. That support was upstreamed and merged.

For the application database archetype MongoDB Realm was selected. Support for it is currently a work in progress as part of the playground project backlog.

#### Apache IoTDB

The Apache IoTDB [project page](https://iotdb.apache.org/) describes IoTDB as:

_"Apache IoTDB (Database for Internet of Things) is an IoT native database with high performance for data management and analysis, deployable on the edge and the cloud. Due to its light-weight architecture, high performance and rich feature set together with its deep integration with Apache Hadoop, Spark and Flink, Apache IoTDB can meet the requirements of massive data storage, high-speed data ingestion and complex data analysis in the IoT industrial fields."_

The diagram below summarizes some of the features that make it an attractive addition to the playground.

{{< figure src="Apache-IoTDB-feature-summary.drawio.svg" title="Summary of Apache IoTDB features" width=50pc >}}

+ The focus on timeseries IoT data delivering high throughput, with low latency are a good fit with the automotive domain. The availability of a single node (Edge) server build is critical for in-vehicle investigations and is supported by the existence of a cluster build for the cloud if needed.

+ Its support for client server, event and streaming data architectures allows for flexible operation using a single solution.

+ It also has very wide API support that greatly enhances the pluggability of the data store into other components, whilst supporting the playground project goal of loose coupling.

+ Similarly the wide range of integrations into other tools and frameworks, particularly the Apache big data stack, simplifies the integration of VSS and associated data into data-centric workflows.

+ IoTDB has a wide range of built-in timeseries data processing and analysis functions covering topics such as data quality, profiling, repair, anomaly detection and series discovery. For example, in-vehicle data analysis can be performed to derive some information or knowledge and then data reduced for further processing in the cloud.

+ As well as Native APIs IoTDB also supports a SQL-like query language. This supports the describing of work in logical terms that affords some portability to other solutions.

## Deployment
The project is currently targeting two deployment scenarios:
1. Easy to develop: make it easy to build, modify and trial by providing a containerized instance running on a host using [Docker containers](https://www.docker.com/).
2. Closer to production: the same base code should be deployable to systems closer to production, including on automotive hardware (or its simulation), e.g. [Yocto](https://www.yoctoproject.org/), container orchestration, [Service-orientated architecture (SOA)](https://en.wikipedia.org/wiki/Service-oriented_architecture) etc.

Docker containers are chosen to facilitate the rapid integration and/or swapping of technology options. Be it internals of the playground itself, or connection to other components. For example, users can easily integrate the playground into their own [docker compose](https://docs.docker.com/compose/) containing other components.

#### Status
The containerized docker deployment is available. Deployment on automotive hardware is currently not but is part of the project backlog.

### Host based Docker

The playground provides docker compose files to deploy the playground using docker images. The deployment contains three main services, for Apache IoTDB, VISSR and Redis.

Please see the playground docker readme.md in the source tree for details.

The service definitions are based on the upstream compose files from the IoTDB and VISSR projects.

### Automotive hardware

The project will provide information as hardware deployment progresses.