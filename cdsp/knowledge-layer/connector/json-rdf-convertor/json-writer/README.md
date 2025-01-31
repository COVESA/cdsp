# JSON Writer

## Overview
The [`JSON Writer`](./src/json_writer.h) module provides functionality to convert different SPARQL query data formats (CSV, TSV, SPARQL JSON, SPARQL XML) into JSON. It also supports storing the generated JSON data in a file.

## Features
- Parses **CSV and TSV** formatted query results.
- Supports **SPARQL JSON and SPARQL XML** formats.
- Stores JSON output in a file if a path is provided.
- Automatically detects and converts data types (integer, float, boolean, string).
- Uses a pluggable file handler for flexible file writing.

## Example Usage
### Converting Data to JSON
You can convert query results to JSON using `JSONWriter::writeToJson()`:
```cpp
#include "json_writer.h"

std::string csv_input = "id,name,age\n1,Alice,30\n2,Bob,25";
nlohmann::json result = JSONWriter::writeToJson(csv_input, DataQueryAcceptType::TEXT_CSV);
std::cout << result.dump(2) << std::endl;
```

### Writing JSON to a File
```cpp
std::optional<std::string> output_path = "./output.json";
nlohmann::json result = JSONWriter::writeToJson(csv_input, DataQueryAcceptType::TEXT_CSV, output_path);
```
This will create a file containing the formatted JSON.

## API Reference
### `JSONWriter::writeToJson`
```cpp
static nlohmann::json writeToJson(
    const std::string& query_result,
    const DataQueryAcceptType& result_format_type,
    std::optional<std::string> output_file_path = std::nullopt);
```
- **query_result**: Input data string.
- **result_format_type**: Format type (`TEXT_CSV`, `TEXT_TSV`, `SPARQL_JSON`, `SPARQL_XML`).
- **output_file_path**: (Optional) Path to store the JSON output file.
- **file_handler**: (Optional) File handler for writing the output, it already has a handler implemented by the system and it can be omitted.
- **Returns**: Parsed JSON object.

## Testing
Unit and integration [tests](./tests/) are provided to ensure the functionality. Test cover the main functionality of `JSONWriter` and error handling.
