# Knowledge Layer "Hello World" Use Case: Driving Style Detector

## Why?

This repository demonstrates a simple "Hello World" application of the Knowledge Layer, whose joint development was [proposed](https://wiki.covesa.global/pages/viewpage.action?pageId=71074417) in the COVESA Data Architecture working group. Given the generic nature of the underlying architecture, this use case can serve as a starting point for implementing more complex scenarios across various domains. One major advantage is that the use case is built on logical [playground components](../../cdsp/README.md). These components can often be swapped out (based on availability of alternatives) without significantly impacting other components (sometimes even not at all, e.g., whether using [IoTDB](../../cdsp/information-layer/handlers/src/iotdb/README.md) or other database component, it does not affect the Knowledge Layer).

### Key Aspects Illustrated:

- **Standardized Data Models:** Using models like [VSS](https://github.com/COVESA/vehicle_signal_specification/) and associated tools enables scalable, generic transformation of data into other formats, like graph data formats.
- **Logic Mapping in Data-Driven Rules:** Instead of hardcoded IF-ELSE logic, data-driven rules like [Datalog](https://en.wikipedia.org/wiki/Datalog) offer:
  - Easier maintenance of logic
  - Functional logic without the need for compilation
  - Smaller size for necessary updates
  - Easier traceability of decisions through declarative logic description
  - Centralized executable knowledge instead of scattered functional logic
- **No Over-Reliance on Machine Learning:** AI is not only Machine Learning, and Machine Learning isn't always the solution.
- **Uniform Implementation Pattern:** Offers a domain-independent pattern for other use cases.

## What?

In this use case, we have implemented an AI-powered solution to detect aggressive driving behaviours. Rather than relying on complex, maintenance-intensive IF-ELSE logic in code or overly sophisticated machine learning models, the detection is achieved through straightforward, data-driven rules.

## How?

Live VSS data from the current drive made accessible in the [Information Layer Server](../../cdsp/information-layer/README.md) via [Websocket](../../cdsp/information-layer/router/src/websocket-server.ts) are converted by a [JSON-RDF-Convertor](../../cdsp/knowledge-layer/connector/README.md) in real-time into a graph data format ([RDF](https://www.w3.org/RDF/)) and stored within the Knowledge Layer in a [Knowledge Graph](https://en.wikipedia.org/wiki/Knowledge_Graph). At any point, every data point (needed for the use case) in the Information Layer has a graph representation. This data representation allows us to attach a [symbolic reasoner](../../cdsp/knowledge-layer/symbolic-reasoner/README.md) ([RDFox](../../cdsp/knowledge-layer/symbolic-reasoner/rdfox/README.md) or [RDF4J](../../../docker/rdf4j/README.md)) to the Knowledge Graph, which can link, evaluate, and infer new facts based on rules, such as deriving the driving style. As soon as an aggressive driving style is detected, the result is converted back from the graph data format to an Information Layer tree format in real time and, in our case, stored in the appropriate data field in the VSS tree. The information "aggressive driving style" can then be shared with other interested applications for example via a data sync middleware.

### Implementation Details

#### Logical Components and Implementation Decisions:

- **Data Model:** [VSS](https://github.com/COVESA/vehicle_signal_specification/) - Describes vehicle data in a standardized format
- **Simulator:** [Remotive Labs](../../examples/remotivelabs-feeder/README.md) - Provides raw data for a test drive
- **DB Handler:** [IoTDB](../../cdsp/information-layer/handlers/src/iotdb/README.md) - Abstracts data store details
- **DB Router:** [WebSocket Server](../../cdsp/information-layer/router/src/websocket-server.ts) - Provides read,
  write and subscribe capabilities to VSS data via a websocket server
- **Connector:** [Knowledge Layer Connector](../../cdsp/knowledge-layer/connector/README.md) - Connects Knowledge Layer to Information Layer via websocket client and manages data tasks within Knowledge Layer
- **Convertor:** [JSON-RDF-Converter](../../cdsp/knowledge-layer/connector/json-rdf-convertor/README.md) - Converts tree-like data (json) into graph data (RDF) and vice versa
- **Reasoner Adapter:** [RDF4J](../../../docker/rdf4j/README.md) (default, open-source) or [RDFox](../../cdsp/knowledge-layer/symbolic-reasoner/README.md) (requires commercial license) adapter - Selected via `model_config.json`; both expose the same REST interface to the Knowledge Layer
- **Rules Language:** [SHACL](https://www.w3.org/TR/shacl/) `sh:SPARQLRule` (RDF4J, default) or [Datalog](https://en.wikipedia.org/wiki/Datalog) (RDFox) - Allows describing IF-ELSE like rules in a data-near language
- **Knowledge Graph and Reasoner:** [RDF4J](../../../docker/rdf4j/README.md) (default) or [RDFox](../../cdsp/knowledge-layer/symbolic-reasoner/rdfox/README.md) (licensed) - Stores the transformed and newly generated graph data, reasons based on rules and graph data, potentially inferring new graph data

![The Use Case in a DIKW,logical and implementation view](KL-example-readme-graphic.png)

#### Config generation

`IL-config/schema-files/vss_data_points.yaml` and both `KL-config/*/shacl/vehicle_shacl.ttl`
copies are generated from a single GraphQL schema rather than hand-maintained (see
[s2dm/README.md](s2dm/README.md)). This runs automatically as part of `docker compose
up` (see step 5 below); you don't need to do anything extra unless you're editing the schema
yourself.

#### Input, Use Case Logic and Output:

Our input data for the use case includes:

- `Vehicle.Chassis.SteeringWheel.Angle`: The current angle of the steering wheel.
- `Vehicle.CurrentLocation.Latitude`: The latitude of the vehicle's current location.
- `Vehicle.CurrentLocation.Longitude`: The longitude of the vehicle's current location.
- `Vehicle.Speed`: The current speed of the vehicle.

The core logic of our use case is represented by the following rule (here in natural language for better readability):

<pre data-toolbar-order="disclaimer,copy-code,show-language" class="language-plaintext" style="border-radius: 0.5rem;"><code class="language-plaintext">If Vehicle.Chassis.SteeringWheel.Angle changes by more than 90 degrees in less than 3 seconds and Vehicle.Speed is greater than 50 km/h, then flag as aggressive driving.</code></pre>

Derived output data:

- `Vehicle.AI.Reasoner.InferenceResults`: Infered driving style details are written into this data node

## Installation and Running

### Remotive Labs and Reasoner Setup

1. Create a [Remotive Labs](https://remotivelabs.com/) account.
2. Navigate to `Recordings`, select `Night drive to Luftkastellet`, and start the recording with the `Play` button. Wait until it starts playing, then stop the recording for now. This creates a broker for you, and we need its credentials next.
3. Navigate to the `Brokers` section and open `My personal broker`.
4. Copy the URL and API key to the environment variables in the [.env](.env) file: `REMOTIVE_LABS_BROKER_URL` and `REMOTIVE_LABS_BROKER_API_KEY`.
5. *(Optional, only for the `rdfox` profile)* Acquire the `RDFox.lic` license file from [Oxford Semantic Technologies](https://www.oxfordsemantic.tech/) and place it in [examples/shared-config/rdfox](../shared-config/rdfox). This step is **not** required for the default RDF4J profile.

### Run Use Case

1. Log in to [Remotive Labs](https://remotivelabs.com/).
2. Select `Night drive to Luftkastellet` in the `Recordings` section, open the `Files` tab, and drag and drop the zip file `examples/shared-config/remotive-labs/config_vss_aggressive_driving.zip` into the `Signal Transformations` section.
3. Navigate to the `Playback` tab, select `config_vss_aggressive_driving` as the `signal transformation`, and press the `Play` button.
4. If the recording starts playing, stop it for now to proceed with setup.
5. Start all services from the current folder ([knowledgelayer-hello-world]()). Make sure you have installed `Docker` and `Docker Compose`.

   ```bash
   docker compose --profile rdf4j up
   ```

   RDF4J is the default reasoner — it is fully open-source and does not require a license. To run with the RDFox reasoner instead (requires a commercial license, see setup step above):

   ```bash
   docker compose --profile rdfox up
   ```

   This first runs `s2dm-config-gen`, a one-shot step that (re)generates the IL/KL config files
   from the GraphQL schema (see [s2dm/README.md](s2dm/README.md)) before anything else starts.
   Nothing new to type, it's part of the same command. If you've edited the schema yourself, add
   `--build` (or run `docker compose build s2dm-config-gen` first) so the change is picked up.

6. Wait until all containers are created and started.
7. Continue playing the `Night drive to Luftkastellet` on `Remotive Labs` page. You should see a lot of logs in the terminal indicating that data is flowing.
8. Knowledge Layer will generate triples in `KL-config/<reasoner>/output/triples` folder and the reasoner responses that are send to Information Layer in `KL-config/<reasoner>/output/reasoning_output`
9. To stop and delete the docker containers you can use
   ```bash
   docker compose --profile rdf4j down
   ```
   or, if you started the RDFox variant:
   ```bash
   docker compose --profile rdfox down
   ```

### Verifying the Setup Without Remotive Labs

`test/feed_test_data.py` exists so you can confirm the stack works end to end without needing a
Remotive Labs account or driving recording. It feeds synthetic signal data straight into the
Information Layer over its websocket API (the same `set`/`get` protocol Remotive Labs' bridge
uses), triggers the driving-style reasoning rules with a steering-angle swing large enough to
qualify as aggressive driving, and reads the result back out to prove the whole loop (including
the S2DM-generated IL/KL config from step 5) actually works, not just that the containers started.

With the stack already running (step 5 above, either profile):

```bash
cd examples/knowledgelayer-hello-world/test
uv run --with websockets python feed_test_data.py
```

A successful run prints `PASS` along with the inferred driving-style result. A failure raises
with a clear error explaining what didn't match, rather than leaving you to guess.

---

## Acknowledgements

Thanks to [David Hoermann](mailto:david.hoermann@campus.tu-berlin.de) for evaluating CDSP, proposing the RDF4J integration approach, and providing the RDF4J rules and queries used by this example.

Feel free to explore the components and rules used in this "Hello World" example to understand the basic implementation of the Knowledge Layer. You can extend this use case to more complex scenarios and even adapt it to different domains.
