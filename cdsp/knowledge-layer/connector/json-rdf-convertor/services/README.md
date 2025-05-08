# Services Module

This module contains service classes responsible for handling interactions with the reasoning backend. These services serve as an abstraction layer to process reasoning using the [Reasoner Service](/cdsp/knowledge-layer/symbolic-reasoner/services/README.md) and process responses with [JSON converter](/cdsp/knowledge-layer/connector/json-rdf-convertor/json-writer/README.md).

## Components

### ReasonerQueryService

Implementation of the `ReasoningQueryService` class, which processes reasoning queries by interacting with a `ReasonerService` and outputs results in JSON format.

The primary purpose of this module is to:
- Handle incoming reasoning queries in various languages (e.g., SPARQL).
- Process those queries using the provided `ReasonerService`.
- Output results as JSON, optionally writing them to a file.

**Example Usage**

To use the `ReasoningQueryService`, instantiate it with a shared pointer to a `ReasonerService` and call `processReasoningQuery()` with the desired query.

```cpp
auto service = std::make_shared<ReasoningQueryService>(reasoner_service);
nlohmann::json result = service->processReasoningQuery({QueryLanguageType::SPARQL, "SELECT * WHERE {?s ?p ?o}"});
```
