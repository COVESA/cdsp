# Reasoner - WebSocket Client Module

## Overview
The `websocket-client` module implements a [WebSocket client](main.cpp) that facilitates communication between external systems and the internal application logic. It supports sending and receiving messages using structured DTOs (Data Transfer Objects) and converting them into BOs (Business Objects) for processing.

## Module Structure
The module is organized into the following subdirectories:

### 1.  **Source** (`src/`)
Contains the core [WebSocket client](main.cpp) implementation, which is the entry point for running the WebSocket client: 
- Manages actual WebSocket connections and message exchange.
- Defines and implements the WebSocket client logic for handling connections, sending requests, and processing responses.

### 2. **Services** (`service/`)
Contains the core service components responsible for DTO-to-BO and BO-to-DTO conversions, message utilities, and schema mapping.

Example conversion flow:

1. **Receiving Data**: External data is received as a DTO (e.g., `DataMessageDTO`).
2. **DTO to BO Conversion**: The DTO is converted into a `DataMessage` BO for internal processing.
3. **Processing Logic**: The `DataMessage` BO executes the required business logic.
4. **BO to DTO Conversion**: The processed BO is converted back into a `DataMessageDTO` for communication.
5. **Sending Data**: The DTO is serialized and sent to the intended recipient (e.g., API response, WebSocket message)

## Design Principles
### **DTO-BO Pattern**
This module follows the DTO-BO pattern to maintain a clear separation between network models and internal business logic.

- **DTOs (Data Transfer Objects)**: Used for structuring messages sent and received over WebSockets, ensuring interoperability with external systems.
- **BOs (Business Objects)**: Used for processing data internally, ensuring consistency and validity before interacting with the application logic.
- **Conversion Services**: The `dto_to_bo` and `bo_to_dto` services handle the transformation between DTOs and BOs seamlessly.

## Usage
To use the WebSocket client, instantiate and configure the `WebSocketClient` class with appropriate connection parameters. The client will handle communication and message processing transparently.

```cpp
WebSocketClient client("ws://server.com:1234");
client.connect();
client.sendMessage("Hello, WebSocket!");
```

# Testing

Several [test](../tests/) are provided to ensure functionality. Test cover the main components (`WebsocketClientIntegrationTest`) to verify the complete reasoning workflow, and (`DTOService`, `DTOToBO`, `BOToDTO`, `BOService`) to ensure data conversion between incoming messages from the external Websocket Server and the core system.


