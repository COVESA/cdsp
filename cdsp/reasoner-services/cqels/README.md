# CQELS Reasoner Service

This folder contains the Java-based CQELS reasoner service. It runs as a Docker container (profile `cqels`) or natively, and is one of the [reasoner services](../README.md) of CDSP.

## Overview

[CQELS](https://github.com/cqels/CQELS4J) is an open-source stream reasoner: continuous queries in CQELS-QL run over time windows of stream elements. The service wraps the `cqels-engine` library (version `2.0.0-alpha.21`, from the Maven repository `https://maven.cqels.org/releases`) in a Spring Boot REST server and provides:

- one CQELS engine per datastore (session), with an in-memory store
- static data (Turtle) and one-shot SPARQL SELECT queries on that store
- continuous CQELS-QL queries, stream elements pushed with their event time, and the query results fetched over REST

The service is stateless across restarts: the Knowledge Layer pushes queries, data and stream elements at runtime.

> [!NOTE]
> The Knowledge Layer adapter for CQELS is not available yet. Until then, the service is used directly over REST, see [Testing](#testing).

## Building

```bash
# Build the Docker image
docker build -t cqels-reasoner .

# Or build with Maven directly (JDK 17)
mvn clean package
```

## Running

```bash
# Run as Docker container
docker run -p 12110:12110 cqels-reasoner

# Or run the JAR directly
java -jar target/*.jar
```

With Docker Compose, see the [Docker README](../../../docker/README.md#cqels-reasoner). There, the variables below are read from `docker/.env`.

## API

The service exposes a REST API on port `12110`:

| Endpoint                                            | Method | Description                                                                                       |
| --------------------------------------------------- | ------ | ------------------------------------------------------------------------------------------------- |
| `/health`                                           | GET    | Returns `OK` when the service is up                                                               |
| `/datastores`                                       | GET    | List all datastores                                                                               |
| `/datastores/{name}`                                | POST   | Create a datastore                                                                                |
| `/datastores/{name}`                                | DELETE | Delete a datastore and stop its engine                                                            |
| `/datastores/{name}/content`                        | POST   | Load static data (Turtle)                                                                         |
| `/datastores/{name}/sparql`                         | POST   | One-shot SPARQL SELECT on the static data; SPARQL JSON if `Accept` contains `json`, otherwise TSV |
| `/datastores/{name}/queries`                        | POST   | Register a continuous query (CQELS-QL, `text/plain`); returns the query id                        |
| `/datastores/{name}/streams/{stream}?eventTime=...` | POST   | Push one stream element (Turtle) observed at `eventTime`                                          |
| `/datastores/{name}/queries/{id}/results`           | GET    | Results of a query since the last call, as SPARQL JSON                                            |

Invalid input returns `400` with the reason, an unknown datastore or query returns `404`.

### Continuous queries

- The query id is the name after `REGISTER QUERY`.
- Register the queries before pushing the first stream element. The streams named in a query are created at registration.
- Each result is returned once: a call to `/results` returns the rows that came in since the previous call.

### Stream elements and event time

- One request is one stream element: all triples of the request belong to it.
- `eventTime` is the observation time in ISO-8601 with an offset, for example `2026-10-30T10:15:30.250Z`. Windows follow the event time, not the time of arrival.

## Configuration

Environment variables:

| Variable                    | Default | Description                                                                                                                    |
| --------------------------- | ------- | ------------------------------------------------------------------------------------------------------------------------------ |
| `SERVER_PORT`               | `12110` | Server port                                                                                                                    |
| `CQELS_RESULTS_BUFFER_SIZE` | `1000`  | Results kept per query until they are fetched. When the buffer is full, the oldest results are dropped and a warning is logged |
| `CQELS_REORDER_TOLERANCE`   | `500ms` | How late an element may arrive and still be used, see [Reorder buffer](#reorder-buffer). `0ms` turns the buffer off            |

### Reorder buffer

Signals reach the service one by one, so an element can arrive after a newer one. Once the engine has processed the newer element, it drops the late one without notice, for example from an aggregate window. In joins, a late element is kept but can miss join partners that have already left the window, so the buffer is needed there too.

The service therefore holds the elements of each stream and pushes them to the engine in event-time order:

- once the stream is the tolerance ahead (a newer element arrived with an event time at least the tolerance later), or
- once they have waited the tolerance in real time, so the last element of a stream does not wait for a newer one.

A step-by-step example with data is in the [service package README](src/main/java/org/covesa/cdsp/cqels/service/README.md#example-reorderbuffer).

Consequences:

- Results arrive up to the tolerance plus 100 ms later than without the buffer.
- An element older than one already pushed to the engine is passed on at once, with a warning in the log. The engine may drop it.
- Deleting a datastore drops the elements it still holds.

## Limitations

Limits of `cqels-engine` `2.0.0-alpha.21` that matter for CDSP:

- **Aggregates need a newer element.** An aggregate over a `[RANGE]` window gives the result of a window only when a newer element arrives. The last window gives no result while no new data comes, for example when the vehicle stops sending. The reorder buffer does not change this.
- **`LATENESS` only works on single-stream `[RANGE]` aggregates.** Joins and plain windowed SELECTs reject it, so the service uses its own reorder buffer.
- **Numbers come back as Java values.** For example, an `xsd:integer` input comes back as `xsd:long`. The value itself is unchanged.
- Other limits of the engine (for example `REPLACE()` and `xsd:dateTime` subtraction unbound) are listed in the CQELS4J issues.

## Design choices

- The query id is the name after `REGISTER QUERY`.
- Results are fetched by polling (`/results`), not pushed to the client.
- The event time is passed as a query parameter per element.
- Late elements are handled by the reorder buffer in the service, not by `LATENESS` in the engine.

## Testing

```bash
# Unit and integration tests
mvn test

# Manual REST test against a running service
./test/run_test.sh
```
