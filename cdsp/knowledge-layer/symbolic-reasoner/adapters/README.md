# Reasoner Integration Guide

This document describes how reasoners are integrated into the Knowledge Layer and provides a step-by-step guide for adding new reasoner backends.

## Architecture Overview

The Knowledge Layer uses a **factory pattern** to support multiple reasoning engines. Each reasoner consists of:

1. **C++ Adapter** — Implements `IReasonerAdapter` interface, handles communication with the external engine
2. **External Engine** — The actual reasoning service (runs as Docker container or external process)
3. **Model Configuration** — JSON config specifying which reasoner to use

## Folder Structure

```
covesa.cdsp/
├── cdsp/knowledge-layer/
│   ├── connector/utils/
│   │   └── data_types.h                    # Enum: InferenceEngineType
│   │
│   └── symbolic-reasoner/
│       ├── CMakeLists.txt                  # Build configuration
│       │
│       ├── adapters/
│       │   ├── rdfox/                      # RDFox adapter
│       │   │   ├── README.md
│       │   │   ├── src/
│       │   │   │   ├── rdfox_adapter.h
│       │   │   │   └── rdfox_adapter.cpp
│       │   │   │
│       │   │   └── tests/                  # Adapter-specific tests
│       │   │
│       │   └── rdf4j/                      # RDF4J adapter
│       │       ├── README.md
│       │       └── src/
│       │           ├── rdf4j_adapter.h
│       │           └── rdf4j_adapter.cpp
│       │
│       ├── interfaces/
│       │   └── i_reasoner_adapter.h        # Common interface all adapters implement
│       │
│       ├── services/
│       │   ├── reasoner_factory.h          # Factory header (includes adapters)
│       │   └── reasoner_factory.cpp        # Factory implementation (instantiates adapters)
│       │
│       └── examples/use-case/
│           ├── rdfox_model/                # Config for RDFox
│           │   └── model_config.json
│           └── rdf4j_model/       # Config for RDF4J
│               └── model_config.json
│
└── docker/
    ├── rdfox/                              # RDFox external engine
    │   └── RDFox.lic
    └── rdf4j/                              # RDF4J external engine
        ├── Dockerfile
        ├── README.md
        ├── pom.xml
        └── src/main/java/...
```

## Currently Supported Reasoners

| Reasoner | Enum Value | Adapter Location | Engine Location |
| -------- | ---------- | ---------------- | --------------- |
| RDFox    | `RDFOX`    | `rdfox/src/`     | `docker/rdfox/` |
| RDF4J    | `RDF4J`    | `rdf4j/src/`     | `docker/rdf4j/` |

---

## Adding a New Reasoner

Follow these steps to add a new reasoning engine (e.g., "MyReasoner"):

### Step 1: Add Enum Value

**File:** `/cdsp/knowledge-layer/connector/utils/data_types.h`

```cpp
enum class InferenceEngineType {
    RDFOX,
    RDF4J,
    MY_REASONER   // Add new reasoner here
};
```

### Step 2: Add String Conversion Functions

**File:** `/cdsp/knowledge-layer/connector/utils/data_types.h`

```cpp
inline InferenceEngineType stringToInferenceEngineType(const std::string& type) {
    std::string lowerCaseType = Helper::toLowerCase(type);
    if (lowerCaseType == "rdfox") {
        return InferenceEngineType::RDFOX;
    } else if (lowerCaseType == "rdf4j") {
        return InferenceEngineType::RDF4J;
    } else if (lowerCaseType == "myreasoner") {
        return InferenceEngineType::MY_REASONER;  // Add conversion
    } else {
        throw std::invalid_argument("Unsupported inference engine: " + type);
    }
}

inline std::string inferenceEngineTypeToString(const InferenceEngineType& type) {
    switch (type) {
        case InferenceEngineType::RDFOX:
            return "RDFox";
        case InferenceEngineType::RDF4J:
            return "RDF4J";
        case InferenceEngineType::MY_REASONER:
            return "MyReasoner";  // Add conversion
        default:
            throw std::invalid_argument("Unsupported inference engine type");
    }
}
```

### Step 3: Create Adapter Class

Create a new folder: `/cdsp/knowledge-layer/symbolic-reasoner/adapters/my-reasoner/src/`

**File:** `/cdsp/knowledge-layer/symbolic-reasoner/adapters/my-reasoner/src/my_reasoner_adapter.h`

```cpp
#ifndef MY_REASONER_ADAPTER_H
#define MY_REASONER_ADAPTER_H

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <string>

#include "data_types.h"
#include "i_reasoner_adapter.h"
#include "request_builder.h"

class MyReasonerAdapter : public IReasonerAdapter {
   public:
    explicit MyReasonerAdapter(const ReasonerServerData& server_data);

    void initialize() override;
    bool loadData(const std::string& data,
                  const std::string& content_type = "text/turtle") override;
    std::string queryData(
        const std::string& query,
        const QueryLanguageType& query_language_type = QueryLanguageType::SPARQL,
        const DataQueryAcceptType& accept_type = DataQueryAcceptType::TEXT_TSV) override;
    bool checkDataStore() override;
    bool deleteDataStore();

   protected:
    virtual std::unique_ptr<RequestBuilder> createRequestBuilder() const;

   private:
    std::string host_;
    std::string port_;
    std::string auth_header_base64_;
    std::string data_store_;
};

#endif  // MY_REASONER_ADAPTER_H
```

**File:** `/cdsp/knowledge-layer/symbolic-reasoner/adapters/my-reasoner/src/my_reasoner_adapter.cpp`

```cpp
#include "my_reasoner_adapter.h"
#include <iostream>
#include <stdexcept>

MyReasonerAdapter::MyReasonerAdapter(const ReasonerServerData& server_data)
    : host_(server_data.host),
      port_(server_data.port),
      auth_header_base64_("Basic " + server_data.auth_base64) {
    if (server_data.data_store_name.has_value()) {
        data_store_ = server_data.data_store_name.value();
    } else {
        throw std::runtime_error("Data store name must be provided.");
    }
}

void MyReasonerAdapter::initialize() {
    std::cout << "** Initializing MyReasoner adapter **" << std::endl;
    // Implementation specific to your reasoner
}

bool MyReasonerAdapter::checkDataStore() {
    // Implementation specific to your reasoner
    return true;
}

bool MyReasonerAdapter::loadData(const std::string& data,
                                  const std::string& content_type) {
    // Implementation specific to your reasoner
    return true;
}

std::string MyReasonerAdapter::queryData(const std::string& query,
                                          const QueryLanguageType& query_language_type,
                                          const DataQueryAcceptType& accept_type) {
    // Implementation specific to your reasoner
    return "";
}

// ... implement remaining methods
```

**File:** `/cdsp/knowledge-layer/symbolic-reasoner/adapters/my-reasoner/README.md`

```markdown
# MyReasoner Adapter

This folder contains the C++ adapter for integrating with the MyReasoner engine.

## Architecture

- C++ Adapter: `adapters/my-reasoner/src/my_reasoner_adapter.h/.cpp`
- External Engine: `docker/my-reasoner/`

## Configuration

Set `"inference_engine": "MyReasoner"` in your model_config.json.
```

### Step 4: Update Factory

**File:** `/cdsp/knowledge-layer/symbolic-reasoner/services/reasoner_factory.h`

```cpp
#include "rdfox_adapter.h"
#include "rdf4j_adapter.h"
#include "my_reasoner_adapter.h"  // Add include
```

**File:** `/cdsp/knowledge-layer/symbolic-reasoner/services/reasoner_factory.cpp`

```cpp
if (inference_engine == InferenceEngineType::RDFOX) {
    reasoner_adapter = std::make_shared<RDFoxAdapter>(server_data);
} else if (inference_engine == InferenceEngineType::RDF4J) {
    reasoner_adapter = std::make_shared<RDF4JAdapter>(server_data);
} else if (inference_engine == InferenceEngineType::MY_REASONER) {
    reasoner_adapter = std::make_shared<MyReasonerAdapter>(server_data);  // Add case
} else {
    throw std::invalid_argument("Unsupported inference engine.");
}
```

### Step 5: Update CMakeLists.txt

**File:** `/cdsp/knowledge-layer/symbolic-reasoner/CMakeLists.txt`

```cmake
add_library(reasoner
    ${CMAKE_CURRENT_SOURCE_DIR}/adapters/rdfox/src/rdfox_adapter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/adapters/rdf4j/src/rdf4j_adapter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/adapters/my-reasoner/src/my_reasoner_adapter.cpp  # Add source
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/request_builder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/services/reasoner_factory.cpp
)

target_include_directories(reasoner
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/adapters/rdfox/src
        ${CMAKE_CURRENT_SOURCE_DIR}/adapters/rdf4j/src
        ${CMAKE_CURRENT_SOURCE_DIR}/adapters/my-reasoner/src  # Add include path
        ${Boost_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/utils
        ${CMAKE_CURRENT_SOURCE_DIR}/services
        ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
)
```

### Step 6: Create Model Configuration

Create folder: `/cdsp/knowledge-layer/symbolic-reasoner/examples/use-case/my_reasoner_model/`

**File:** `/cdsp/knowledge-layer/symbolic-reasoner/examples/use-case/my_reasoner_model/model_config.json`

```json
{
  "inputs": {
    "Vehicle_data": "inputs/vehicle_data_small.ttl"
  },
  "ontologies": ["ontologies/vehicle.ttl"],
  "queries": {
    "triple_assembler_helper": {
      "Vehicle": ["queries/data_property.rq", "queries/object_property.rq"]
    },
    "reasoning_output_queries_path": "queries/",
    "output": "queries/output/"
  },
  "rules": ["rules/driving_style_inference_rules.shacl"],
  "shacl": ["shacl/vehicle_shacl.ttl"],
  "reasoner_settings": {
    "inference_engine": "MyReasoner",
    "output_format": "turtle",
    "supported_schema_collections": ["vehicle"]
  }
}
```

### Step 7: Create a .gitignore for output generated files

This will prevent debug generated triples from being committed to the repository.

**File:** `/cdsp/knowledge-layer/symbolic-reasoner/examples/use-case/my_reasoner_model/.gitignore`

```gitignore
# Ignore the output folder where the reasoner generates new triples
/output/
```

### Step 8: (Optional) Add External Engine

If your reasoner runs as an external service:

**Folder:** `docker/my-reasoner/`

```
docker/my-reasoner/
├── Dockerfile
├── README.md
└── ... (engine-specific files)
```

---

## Runtime Configuration

### Environment Variables

| Variable                   | Description               | Default                                                   |
| -------------------------- | ------------------------- | --------------------------------------------------------- |
| `MODEL_CONFIGURATION_PATH` | Path to model_config.json | `/symbolic-reasoner/examples/use-case/my_reasoner_model/` |
| `HOST_REASONER_SERVER`     | Reasoner server host      | `127.0.0.1`                                               |
| `PORT_REASONER_SERVER`     | Reasoner server port      | `12110`                                                   |
| `REASONER_DATASTORE_NAME`  | Data store name           | `ds-test`                                                 |

### Usage

```bash
# Use RDF4J reasoner
export MODEL_CONFIGURATION_PATH=/symbolic-reasoner/examples/use-case/rdf4j_model/
./reasoner_client

# Use RDFox reasoner (default)
./reasoner_client

# Use custom reasoner
export MODEL_CONFIGURATION_PATH=/symbolic-reasoner/examples/use-case/my_reasoner_model/
./reasoner_client
```

---

## Interface Reference

All adapters must implement `IReasonerAdapter`:

```cpp
class IReasonerAdapter {
public:
    virtual void initialize() = 0;
    virtual bool loadData(const std::string& data,
                          const std::string& content_type) = 0;
    virtual std::string queryData(const std::string& query,
                                   const QueryLanguageType& type,
                                   const DataQueryAcceptType& accept) = 0;
    virtual bool checkDataStore() = 0;
    virtual ~IReasonerAdapter() = default;
};
```

## Testing

After adding a new reasoner:

1. Build the project: `cd build && cmake .. && make`
2. Start the external engine (if applicable)
3. Run with your config: `MODEL_CONFIGURATION_PATH=... ./reasoner_client`
4. Verify initialization and query execution
