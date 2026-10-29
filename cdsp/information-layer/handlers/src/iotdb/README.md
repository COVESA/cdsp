# IoTDB

This directory contains the IoTDB handler implementation for the Information Layer.

The handler now uses the **Node.js native IoTDB client** (`@iotdb/client`) as the only runtime path. Legacy Thrift-based runtime code has been removed.

## Current Architecture

### `IoTDBHandler`

- Main orchestrator for get/set/subscribe/unsubscribe requests.
- Owns a single shared `IoTDBSession` instance.
- Initializes schema and database readiness.
- Delegates subscription lifecycle operations to `SubscriptionSimulator`.

### `IoTDBSession`

- Encapsulates IoTDB session lifecycle (`open`, `close`).
- Executes query/write operations for:
  - `getDataPoints`
  - `getDataPointsInWindow` (polling window support)
  - `setDataPoints`
- Handles database creation via `createDatabaseIfNeeded`.
- Uses explicit SQL dialect setup (`TREE`) when possible and logs fallback if unsupported.

### `SubscriptionSimulator`

- Manages in-memory websocket subscriptions.
- Owns timer lifecycle (start/stop polling interval).
- Uses the **shared** `IoTDBSession` injected by `IoTDBHandler` (does not own/close the session).
- Polls IoTDB using time windows and sends updates to subscribed clients.

## Subscription Model

Node client support for native topic consumer APIs is limited for our target stack, so subscription behavior is currently implemented with polling:

- periodic interval from config (`IOTDB_POLL_INTERVAL_LEN_IN_SEC`)
- checks datapoint changes in `(previous_timestamp, current_timestamp]`
- sends data messages only when changes are found
- unsubscribe and unsubscribe-client cleanup stop polling when no subscriptions remain

## Configuration

The handler reads these variables from `docker/.env`. Create it from the template with `cp docker/.env.example docker/.env`; the template already contains working values for the Docker Compose setup.

| Variable                         | Default               | Meaning                                            |
| -------------------------------- | --------------------- | -------------------------------------------------- |
| `HANDLER_TYPE`                   | none, required        | Must be `iotdb` to use this handler                |
| `IOTDB_HOST`                     | `iotdb-service`       | IoTDB host                                         |
| `IOTDB_PORT`                     | `6667`                | IoTDB port                                         |
| `IOTDB_USER`                     | `root`                | IoTDB user                                         |
| `IOTDB_PASSWORD`                 | `root`                | IoTDB password                                     |
| `IOTDB_FETCH_SIZE`               | `10000`               | Rows fetched per query                             |
| `IOTDB_TIME_ZONE_ID`             | time zone of the host | Time zone of the IoTDB session                     |
| `IOTDB_POLL_INTERVAL_LEN_IN_SEC` | `0.2`                 | Seconds between two checks for changed data points |

The defaults are set in [`config/database-params.ts`](config/database-params.ts).

> [!IMPORTANT]
> On every poll, subscribers only receive the **latest** value of each data point that changed since the previous poll; values in between are not sent. Keep the poll interval well below the time windows of the reasoning rules (the driving-style rules look at changes within 3 seconds), otherwise they never see the changes they look for.

> [!WARNING]
> Do not commit `docker/.env`; it is not tracked in git.

### Running

The IoTDB handler is started by the Websocket-Server (see project root README).
No manual standalone startup is required for normal project execution.

### Validation

Before merging changes in this area, run:

```shell
npm test
npm run build
```
