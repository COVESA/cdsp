# RDF4J Reasoner Service

This folder contains the Java-based RDF4J reasoner service. It runs as a Docker container (profile `rdf4j`) or natively, and is one of the [reasoner services](../README.md) of CDSP.

## Overview

The RDF4J reasoner is a semantic reasoning engine built on Eclipse RDF4J. It provides:

- SPARQL query execution
- RDF data storage (in-memory MemoryStore)
- SHACL rule-based inference via `sh:rule` / `sh:construct` patterns
- The service is stateless, all ontologies, data, and rules are pushed by the Knowledge Layer at runtime

## Architecture

This Java service is consumed by the C++ adapter located at:

```
cdsp/knowledge-layer/symbolic-reasoner/adapters/rdf4j/src/
```

## Building

```bash
# Build the Docker image
docker build -t rdf4j-reasoner .

# Or build with Maven directly
mvn clean package
```

## Running

```bash
# Run as Docker container
docker run -p 12110:12110 rdf4j-reasoner

# Or run the JAR directly
java -jar target/*.jar
```

## API

The service exposes a REST API on port `12110`:

| Endpoint                         | Method | Description                      |
| -------------------------------- | ------ | -------------------------------- |
| `/datastores`                    | GET    | List all data stores             |
| `/datastores/{name}`             | POST   | Create a data store              |
| `/datastores/{name}/content`     | POST   | Load RDF data                    |
| `/datastores/{name}/sparql`      | POST   | Execute SPARQL query             |
| `/datastores/{name}/rules/add`   | POST   | Append SHACL rules (shape graph) |
| `/datastores/{name}/rules/reset` | POST   | Clear all rules and reload       |
| `/datastores/{name}`             | DELETE | Delete a data store              |

## Configuration

Environment variables:

- `PORT`: Server port (default: 12110)

## SPARQL Evaluation Mode

RDF4J supports two evaluation modes. This service uses the default: **strict**.

- **strict** — minimal W3C SPARQL 1.1 compliance, no additional operators.
- **standard** — adds practical extensions (e.g. calendar arithmetic) while remaining W3C-compliant.

> [!NOTE]
> Operations like `xsd:dateTime` subtraction and `xsd:duration` comparisons are XPath 2.0 extensions and are **not available in strict mode**. Use `standard` mode or avoid them in SPARQL queries targeting this service.

Reference: [RDF4J - Tweaking the query evaluation mode](https://rdf4j.org/documentation/programming/repository/#tweaking-the-query-evaluation-mode)
