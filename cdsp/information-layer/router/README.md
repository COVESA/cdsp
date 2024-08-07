With this component, you can configure which database it connects to.

> [!WARNING]
> While configuring Docker for environment variable definition, the `dotenv` package is used to load environment variables from an `.env` file into `process.env`.

# Install

Execute in this directory:

```bash
npm install
```

# Run

In the `.env` file are defined the environment variables:

- **HANDLER_TYPE**: Configure `realmdb` to start the RealmDB Handler or `iotdb` to start the IoTDB Handler.

> [!WARNING]
> Create a `.env` file in the same directory as your `docker-compose.yml`. If it doesn't exist, create it in the same directory as `websocket-server.js`.

Start router by executing in [src](./src/) directory the command:

```bash
node websocket-server.js
```
