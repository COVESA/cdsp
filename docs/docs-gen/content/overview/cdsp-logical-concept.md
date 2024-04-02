---
title: "Playground logical concept"
weight: 20
---

## Importance of maintaining a logical concept
COVESA is a grass roots OSS automotive alliance. From a communication and community perspective it is important to maintain descriptions of the logical concepts. Discussion at a logical level allows different parties to collaborate on common concepts, whilst making different implementation decisions, e.g. in product/technology selection or system architecture for example. That however does not mean we need spend months in philosophical discussions before moving to implementation. Instead logical concepts (why, what) can be developed alongside implementation.

The Data Architecture team therefore intends to maintain concepts for both logical and implementation alongside each other for their projects using the playground.

Of course it is perfectly normal that in _using_ the playground to develop an idea, example or pattern, that an architecture and components are chosen as part of the design.

## Why _Central Data Service Playground_ ?

### Problem
OEMs have presented various open questions and requirements in tackling growing software complexity. One representative list appears below.

"Key questions for a End-to-End Data Architecture:
- How data can be shared between all touchpoints?
- How different domains of data share same tech
stack?
- How should a bidirectional sync work?
- Who is responsible for conflict management?
- Who takes care about permissions, roles, rights
and privacy?
- How the data model can be updated and synced?
- How subscriptions can be handled?
- How to handle historized and time series data ...
- ...on different touchpoints?
- How to handle multiple sync endpoints?
- How to handle unidirectional data streams?
- How (new) knowledge ...
- ... can be shared with others?"

Source: [_"Building Bridges with a common Data Middleware"_, OEM, COVESA Autumn 2023 AMM](https://wiki.covesa.global/download/attachments/78840403/COVESA_DataMiddleware-PoC-Status_v2.pdf?version=1&modificationDate=1698737955726&api=v2)

At the same time there has been a realization that some problems require open collaboration.

### Goal
The playground was first conceived by the COVESA Data Architecture team to meet their needs in addressing these problems. They also recognized similar needs in the wider community inside and outside COVESA. For example, for COVESA to demonstrate how VSS data can be used with its eco-system for newcomers.

The playground goals from the introductory overview:

_The Central Data Service Playground (CDSP) serves as a neutral, open playground for data services both within and outside the vehicle in the context of data-centric architectures. It enables investigation into the internals of these services and how they can be combined. Furthermore, the playground provides a means to publish and collaborate on such work in the open._

The following sections address the Why, What and How in more detail.

### Why _Central Data Service_?

+ VSS is a mechanism of abstraction. The COVESA logical architecture for the VSS eco-system shown below places operation in the 'big ECUs', in zonal ECUs and above. Discussion of next-gen and data-centric architectures suggests investigation into data services in zone, domain and HPC controller scenarios and the cooperation between them. Hence _Central_.

{{< figure src="Logical-Architecture-Overview.drawio.svg" title="The COVESA Logical Architecture" width=50pc >}}

+ A repeating pattern of discussion in the COVESA Data Architecture team is the combination of VSS Data Server and VSS Data Store with advanced features and their connection southbound to feeders/native data and northbound to clients and off-board. Hence _Data Service_.

+ The name Central Data Service is not an attempt to introduce a new category of component. It is used here simply as a useful synonym for what otherwise would be a longer descriptive phrase explaining combinations of VSS centric Data Server and Store and their location in the vehicle.

 ### Why _Playground_? Why not PoC?

+ A PoC is often a snapshot in time and often specific in scope. Playground suggests greater flexibility. The central service, the Lego building block, is intended to be flexible and evolving. Similarly with what it is combined with to illustrate COVESA concepts and technology.

+ The Playground could certainly be used to implement a PoC.

+ Patterns such as view/controller, out of the box data servers, data stores linked to applications (e.g. SQLite) etc are well known. The Service could in part be defined by what's not known, by open questions in next-gen architectures such as data-centric architectures, that needs to be tackled down. The Playground is the means to doing that and illustrating the results.

## What?

### Requirements

{{< figure src="cdsp-logical-concept.drawio.svg" title="The data service core requirements" >}}

At its core the service has requirements in three key areas:

1. Data Models: the live data models - VSS as the abstracted view of the vehicle, along with other adjacent data models such as personal data.

2. Persistence: history of the model and signals etc - historical and cached timeseries data.

3. Application logic / APIs: standardized APIs for accessing the data such as the [Vehicle Information Service Specification (VISS)](https://github.com/COVESA/vehicle-information-service-specification)
 or GraphQL.

_Additional requirements:_ It is recognized that additional features, such as synchronization are absolutely desirable and have been a part of discussions in the Data Architecture team. Such features may also already be part of the feature set of the playground components. For example, both the Apache IoTDB and MongoDB Realm databases have sync capabilities. A base feature set is described as a starting point to help readers quickly grasp the concept. Additional features will be created or illustrated collectively based on interest and participation.

### Components

As a starting point the playground has been realized as a 'project of projects' to create the basic building block data service. Achieved by combining a VISS Data Protocol Server with highly functional VSS Data Stores.

The VISS Protocol Server principally provides northbound get/set/sub application logic using the VISS protocol. Along with the secondary benefit of any additional features provided by the server.

'Highly functional' in the context of the VSS Data Store means the flexible means to store, query, process and analysis timeseries data. An archetype would be a database server that can operate in-vehicle and the cloud. Such a component provides the possibility of using a variety of different architecture patterns, such as both client-server and event-driven, or data processing approaches.

### Flexibility in use

As well as the mentioned flexibility in implementation, flexibility in use is also intended:

+ With some supporting documentation it can help meet the ongoing request from newcomers to the VSS eco-system as to how VSS can be used.

+ The Data Architecture group has various topics it wishes to investigate related to data-centric architectures. Data layer topics such as sync, data reduction and data quality. Also separation of concerns and cooperation between data and knowledge layers.

+ The playground can be used to investigate internals of data services. For example, connecting to medium and high speed data, or adding a protocol.

+ External connections with other systems may also be a focus. For example, combining the service with other components to implement a particular touchpoint such as mobile.

Further details can be found in the following sections.

### The Playground in context

To help understand its use lets quickly place the playground in context.

**Big picture**: As stated above the COVESA logical architecture places its scope at Zonal ECUs and above as shown in the diagram. It is assumed that the playground would likely be deployed on a Zone, Domain or Central controller, with corresponding h/w capabilities.

**Interaction between 'Large ECU'**: In the COVESA Data Architecture team it is recognized that zone/domain specific data services will need to synchronize and cooperate between themselves and/or with a central vehicle computer, e.g. Inter-controller sync/cooperation:

![Cooperation between zonal data stores and central data service/store](https://raw.githubusercontent.com/slawr/vss-otaku/master/in-vehicle-storage/apache-iotdb/doc/zonal-vss-store-cooperation.drawio.svg "Cooperation between zonal data stores")
Source: [vss-otaku](https://github.com/slawr/vss-otaku)

**In-vehicle southbound**: The playground can be integrated southbound to lower parts of the vehicle and its native data, through data feeders and connectors. This can include making connections to other systems such as Autosar etc.

**Northbound**: connections will be made to clients, mobile, cloud and major in-vehicle domains such as IVI running Android/Apple etc.

**Logical domains**: Connections may also be made to other logical data domains. For example, there is an [knowledge layer proposal](https://wiki.covesa.global/x/cYI8B) made in the COVESA Data Architecture team that discusses the separation of concerns and interaction between knowledge, information and raw data layers as illustrated below. The Playground here could be used to provide the data/information layer services in its investigation.

![Knowledge layer proposal](https://wiki.covesa.global/download/attachments/71074417/Knowledge-Layer-v3.png?api=v2?width=50pc "Deployment illustration from the proposal")
  Source: [knowledge layer proposal](https://wiki.covesa.global/x/cYI8B)

### Project success factors

1. Newcomers to COVESA technology use the playground to accelerate their understanding of how the technology can be used. That could be a looking at a simple instance of how a VSS data server is combined with a VSS data store and queried using VISS. It could also be a more complex instance that combines components to illustrate a longer specific end to end use case, e.g. mobile to vehicle connection.

2. Internal groups within COVESA naturally use the logical concepts and the playground implementation in combination with other components to develop and disseminate ideas. This especially applies to the Data Architecture and Infrastructure pillar.

3. Supporting materials such as patterns, diagrams, cookbooks etc are adopted as useful assets within and outside COVESA, which in turn helps socialization.

## How?

+ Address two high level implementation needs, keeping in mind a path towards production where possible:

  - Easy to develop: make it easy to build, modify and trial by providing an instance running on a host, e.g. using Docker container(s).

  - Closer to production: the same base code should be deployable to systems closer to production, including on automotive hardware (or its simulation), e.g. Yocto, container orchestration, SOA etc.

+ A path towards production can be supported by using production components, rather than overly simple substitutes, where it makes sense. For instance a particular scenario may use Kafka in a cloud connection. The point is not to pick a winning product in a particular category, but to recognize that using a production tool can represent a category that is known to scale. Detailed requirements for a specific production project and product selection for it, is rightly left to that project.

+ A generic code base for the basic building blocks should allow flexible compilation to meet those needs on multiple architectures, e.g. x86, ARM and RISC-V. The target for how it is used being a matter of deployment at a high level.

+ Follow the OSS mantra of adopt where you can, extend if needed, create where necessary.

+ Promote flexible reconfiguration of components by favouring loose coupling over tight coupling.