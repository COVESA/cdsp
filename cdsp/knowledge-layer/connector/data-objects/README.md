# Data Objects Module

The `data_objects` module in this project implements a structured approach using the **Data Transfer Object (DTO) and Business Object (BO) pattern**. This approach ensures a clean separation between data representation for communication (DTO) and internal domain logic (BO). This separation is crucial for maintaining a scalable, maintainable, and testable codebase.

### Why Use DTOs and BOs?

- **DTO (Data Transfer Object)**: Primarily used for data exchange between layers (e.g., API, external systems, serialization).
  - Ensures that external dependencies do not leak into the domain model.
  - Simplifies data serialization/deserialization.
  - Reduces unnecessary data exposure by structuring only what is required.
- **BO (Business Object)**: Represents the internal domain model, encapsulating business logic.
  - Ensures encapsulation of business rules.
  - Keeps internal logic clean and independent of data exchange formats.
  - Facilitates better unit testing and maintainability.

## Module Structure

The `data_objects` module is divided into two main subdirectories:

### 1. **Business Objects** (`bo/`)

This directory contains C++ classes that represent the core business logic of the application. Each BO is designed to encapsulate logic related to a specific domain entity. Key components include:

- `data_message.h/.cpp`: Represents structured data messages with internal logic.
- `get_message.h/.cpp`: Handles retrieval of structured information.
- `metadata.h/.cpp`: Manages metadata related to messages.
- `node.h/.cpp`: Represents nodes within structured messages.
- `status_message.h/.cpp`: Encapsulates response status handling.
- `subscribe_message.h/.cpp`: Supports subscription-related business logic.

Each of these files defines the corresponding domain logic required for processing messages and business rules.

### 2. **Data Transfer Objects** (`dto/`)

This directory contains lightweight objects designed for data exchange, typically used for API communication or serialization. Key components include:

- `data_message_dto.h`: Defines DTO structure for data messages.
- `get_message_dto.h`: Defines DTO structure for retrieving data.
- `metadata_dto.h`: Defines DTO structure for metadata.
- `status_message_dto.h`: Defines DTO structure for status messages.
- `subscribe_message_dto.h`: Defines DTO structure for subscription messages.

### How We Use DTO-BO Conversion

To ensure smooth interaction between DTOs and BOs, we implement conversion functions (see [DTO and BO services](../websocket-client/README.md#2-service)). These functions transform incoming DTOs into BOs before processing business logic and convert BOs back into DTOs when sending responses.



