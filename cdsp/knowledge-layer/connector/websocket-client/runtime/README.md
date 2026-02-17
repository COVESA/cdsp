# WebSocket Runtime Utilities

The runtime directory contains lightweight utilities that maintain state across
the WebSocket client’s lifetime. At present it hosts the `RequestRegistry`, a
central component for tracking in-flight subscribe/get/set/unsubscribe
operations and correlating responses.

## RequestRegistry

Defined in `request_registry.*`, this class assigns numeric identifiers to
outgoing requests and stores their metadata:

- **Tracked types** – Standard subscribe/unsubscribe/get/set
  flows.
- **Lookup helpers** – Retrieve request details (`getRequest`), search for a
  request by its metadata (`findRequestId`), and remove entries once handled.

`RequestInfo::typeToString` provides human-readable labels for logging.

## Usage in Services

`RequestRegistry` is injected into several services under
`websocket-client/services/`, including `MessageService`, converters, and
status handlers. Any component that enqueues outbound messages should register
the request to enable response matching.

## Testing

The registry is exercised indirectly through [service tests](../services/tests/) that rely on
request tracking.
