# DB Router

Provides information layer data in tree-like format. Abstracts data store details.

This project is a [WebSocket server](src/websocket-server.ts) that dynamically integrates different database handlers (e.g., RealmDB, IoTDB) based on the [configuration](../handlers/src/README.md). It listens for incoming WebSocket connections, processes messages according to the specified handler, and broadcasts messages to connected clients.
