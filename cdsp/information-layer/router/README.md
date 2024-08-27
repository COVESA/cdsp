With this component, you can configure which database it connects to.

> [!WARNING]
> While configuring Docker for environment variable definition, the `dotenv` package is used to load environment variables from an `.env` file into `process.env`.

# Install

Execute in this directory:

```bash
npm install
```

# Run
The `.env` file contains key-value pairs of environment variables that are used during the build and runtime of the Docker container. This allows for easy configuration of the application without changing the source code.

- **HANDLER_TYPE**: Configure `realmdb` to start the RealmDB Handler or `iotdb` to start the IoTDB Handler.
- **VERSION**: This variable is automatically incremented by the build script, ensuring that every Docker image build has a unique version identifier.

> [!WARNING]
> Create a `.env` file in the same directory as your `docker-compose.yml`. If it doesn't exist, create it in the same directory as `websocket-server.js`.

Start router by executing in [src](./src/) directory the command:

```bash
node websocket-server.js
```
