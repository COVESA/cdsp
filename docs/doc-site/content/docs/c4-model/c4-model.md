---
title: "C4 model of the Software Architecture"
linkTitle: "Software architecture model"
---
## Introduction
CDSP uses the [C4 model](https://c4model.com/) for visualising the software architecture. C4 was adopted as it is easy to learn, developer friendly and is intended to be readable by the widest possible audience.

If you are not familiar with the C4 model 5-10 minutes spent reading the landing page of the [C4 website](https://c4model.com/) will quickly get you up to speed on what you need to know.

Briefly, C4 is an abstraction-first approach to diagramming architectures. A hierarchy of abstractions are used to describe the static structure of a system: *software systems*, *containers*, *components*, and *code*. Each of which can be visualised with a hierarchy of corresponding diagrams: *system context*, *containers*, *components*, and *code*.

![Hierarchy of C4 Model static diagrams](https://c4model.com/images/c4-static.png "Hierarchy of C4 Model static diagrams (C4 website)")

Note: The C4 Model predates Docker and defines a different meaning to the term *container*. In C4 a container represents an application or data store.

## C4 Model Static Diagrams
### Level 1: system context diagram
The CDSP system context diagram below shows CDSP in context with other systems:
![CDSP system context diagram](https://raw.githubusercontent.com/COVESA/cdsp/refs/heads/main/diagrams/CDSP-C4-Model-System-Context-diagram.drawio.svg "CDSP system context diagram: Top level abstraction showing CDSP in context with other systems")

### Level 2: container diagram
The CDSP container diagram below shows the C4 containers (applications and data stores) that make up the CDSP software system:
![CDSP container diagram](https://raw.githubusercontent.com/COVESA/cdsp/refs/heads/main/diagrams/CDSP-C4-Model-Container-diagram.drawio.svg "CDSP container diagram: Second level abstraction showing the C4 model containers within the CDSP core")

### Level 3: component diagrams
The Information (IL) and Knowledge Layer (KL) Server component diagrams below show the components within the IL and KL containers:
![IL/KL component diagrams](https://raw.githubusercontent.com/COVESA/cdsp/refs/heads/main/diagrams/CDSP-C4-Model-KL-IL-Component-diagrams.drawio.svg "IL and KL component diagrams: Third level abstraction showing the components within the Information and Knowledge Layer Servers")

