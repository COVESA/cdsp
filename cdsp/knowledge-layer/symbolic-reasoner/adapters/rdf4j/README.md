# RDF4J Adapter

This folder contains the C++ adapter for integrating with the RDF4J reasoner engine.

## Architecture

The RDF4J integration follows a clean separation of concerns:

```
/cdsp/knowledge-layer/symbolic-reasoner/adapters/rdf4j/   # C++ adapter (this folder)
└── src/
    ├── rdf4j_adapter.h
    └── rdf4j_adapter.cpp

/docker/rdf4j/                        # Java RDF4J engine (external service)
├── Dockerfile
├── pom.xml
└── src/main/java/...
```

### C++ Adapter (`src/`)

The `RDF4JAdapter` class implements the `IReasonerAdapter` interface, providing:

- HTTP-based communication with the RDF4J server
- SPARQL query execution
- Data store management
- RDF data loading

### Java Engine (`docker/rdf4j/`)

The RDF4J reasoner runs as an external Docker service. The Java implementation is maintained separately in `docker/rdf4j/` and communicates with this adapter via REST API.

## Building

The adapter is built as part of the Knowledge Layer CMake project:

```bash
cd cdsp/knowledge-layer
mkdir build && cd build
cmake ..
make
```

## Configuration

Set the inference engine to `RDF4J` in your `model_config.json`:

```json
{
  "reasoner_settings": {
    "inference_engine": "RDF4J"
  }
}
```

## Running the RDF4J Service

```bash
cd docker/rdf4j
docker build -t rdf4j-reasoner .
docker run -p 12110:12110 rdf4j-reasoner
```
