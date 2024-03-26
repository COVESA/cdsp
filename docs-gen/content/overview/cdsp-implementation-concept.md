---
title: "Playground implementation"
weight: 30
---

{{% notice warning %}}
This placeholder text comes from the Playground Proposal and will be replaced with a fuller discussion of the implementation
{{% /notice %}}

## Implementation Concepts

The logical section above should be read to understand the concepts to be implemented. This section suggests starting points for discussion in the community.
Initial idea for the Central Data Service

As outlined in the logical description, as a starting point the Service could be realised as a basic building block combining VISS Data Server with highly functional VSS Data Store. The availability of generic code allows flexible deployment to meet the two high level implementation needs to support easy development trials, whilst also supporting investigation closer to production, including a path to production.

Initial idea/sketch for base building block:
 - Generic code: VISS Data Server with VSS Data Store backend
   - Data Architecture requirements: Add Apache IotDB (Apache eco-system, embedded and UDF) and Realm (embedded, sync) as backends to enable research.
   - Example using WAII VISS Data Protocol Server, which supports historical data and has a data store backend and various embedded databases:
 - Deployment 1 - Easy to develop (assumed first target): x86 host Docker containers
 - Deployment 2 - Closer to production (assumed second target): ARM64 using common automotive deployment, e.g. Yocto Linux.