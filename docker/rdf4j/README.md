# RDF4J Reasoner Service

This folder contains the Java-based RDF4J reasoner service that runs as an external Docker container.

## Overview

The RDF4J reasoner is a semantic reasoning engine built on Eclipse RDF4J. It provides:

- SPARQL query support
- RDF data storage and management
- Inference capabilities

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

| Endpoint                     | Method | Description          |
| ---------------------------- | ------ | -------------------- |
| `/datastores`                | GET    | List all data stores |
| `/datastores/{name}`         | POST   | Create a data store  |
| `/datastores/{name}/content` | POST   | Load RDF data        |
| `/datastores/{name}/sparql`  | POST   | Execute SPARQL query |

## Configuration

Environment variables:

- `PORT`: Server port (default: 12110)
