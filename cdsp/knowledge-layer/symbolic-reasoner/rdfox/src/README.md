# RDFoxAdapter

## Overview

`RDFoxAdapter` is responsible for communicating with the RDFox server using RESTful APIs. This adapter allows users to perform various operations such as creating connections, loading data, querying data (using [SPARQL](https://www.w3.org/TR/sparql11-query/)), and managing cursors efficiently.


> **Note:** When the `RDFoxAdapter` initializes, it creates a datastore called `vehicle_ds` in the RDFox server if it does not exist.


## Features


- **Data Store Management**:
  - Initialize and ensure the existence of the datastore.
  - Load data into the datastore in various formats (e.g., Turtle, JSON-LD, RDF/XML).
  - Delete the datastore.

- **Data Querying**:
  - Query data using SPARQL queries.
  - Support for multiple response content types such as `table/csv`, `application/sparql-results+json`, etc.

- **Connection Management**:
  - Create and manage connections to the RDFox datastore.
  - Validate existing connections.

- **Cursor Management**:
  - Create cursors for large query results.
  - Advance or open cursors for efficient pagination.
  - Delete cursors after usage.

## Example Usage

**Initializing the Adapter**

```cpp
#include "rdfox_adapter.h"

int main() {
    RDFoxAdapter adapter("<rdf_host>", "<port>", "<auth>", "<datastore>");

    try {
        adapter.initialize();
        std::cout << "Datastore initialized successfully!" << std::endl;
    } catch (const std::runtime_error& e) {
        std::cerr << "Error initializing datastore: " << e.what() << std::endl;
    }

    return 0;
}
```

**Loading Data**

```cpp
std::string turtle_data = R"(
    @prefix ex: <http://example.com/> .
    ex:subject a ex:Object .
)";
adapter.loadData(turtle_data, ReasonerSyntaxType::TURTLE);
```

**Querying Data**

```cpp
std::string sparql_query = "SELECT ?s ?p ?o WHERE { ?s ?p ?o }";
std::string result = adapter.queryData(sparql_query, "application/sparql-results+json");
std::cout << "Query Result: " << result << std::endl;
```

**Managing Cursors**
```cpp
std::pair<std::string, std::string> connection = adapter.createConnection();
std::string cursor_id = adapter.createCursor(connection.first, connection.second, sparql_query);

std::string cursor_result;
adapter.advanceCursor(connection.first, connection.second, cursor_id, 
                      "application/sparql-results+json", "open", 100, &cursor_result);

std::cout << "Cursor Result: " << cursor_result << std::endl;

adapter.deleteCursor(connection.first, cursor_id);
```

> [!NOTE] Allowed Operations for Advancing the Cursor
> The `RDFoxAdapter::advanceCursor` method allows two operations:
> - **`open`:** Opens the cursor for the first time and retrieves data starting from the beginning.
> - **`advance`:** Advances the cursor from its current position to the next set of results.

## Supported Data Formats

### For Loading Data

- text/turtle
- application/ld+json
- application/n-triples
- application/n-quads

### For Query Responses

- table/csv
- text/plain
- application/sparql-results+json
- application/sparql-results+xml

# Testing

Unit and Integration [tests](../tests/) are provided to ensure functionality. Tests cover the main components (`RDFoxAdapter`) to verify RDF data transfer, and error handling.
