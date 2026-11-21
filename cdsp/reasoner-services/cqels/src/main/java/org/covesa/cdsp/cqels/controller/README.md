# Controllers

The REST endpoints of the CQELS reasoner service. Each controller turns HTTP requests into calls on the `service` package and turns the answers into HTTP responses. The full API with request and response formats is in the [service README](../../../../../../../../README.md#api).

```mermaid
%%{
  init: {
  "theme": "base",
  "themeVariables": {
    "primaryColor": "#393552",
    "primaryTextColor": "#e0def4",
    "primaryBorderColor": "#6e6a86",
    "lineColor": "#908caa",
    "secondaryColor": "#26233a",
    "tertiaryColor": "#e0def4",
    "fontFamily": "monospace",
    "background": "#26233a",
    "clusterBkg": "#26233a",
    "clusterBorder": "#6e6a86"
  }
}
}%%
graph LR;
    CLIENT(Knowledge Layer or REST client)

    subgraph controller
        SESSION_CONTROLLER(SessionController)
        STATIC_DATA_CONTROLLER(StaticDataController)
        STREAM_CONTROLLER(StreamController)
    end

    subgraph service
        SESSION_REGISTRY(SessionRegistry)
        SESSION(Session)
    end

    ENGINE(CQELSEngine)

    CLIENT -- rest --> SESSION_CONTROLLER
    CLIENT -- rest --> STATIC_DATA_CONTROLLER
    CLIENT -- rest --> STREAM_CONTROLLER
    SESSION_CONTROLLER ----> SESSION_REGISTRY
    STATIC_DATA_CONTROLLER ----> SESSION_REGISTRY
    STREAM_CONTROLLER ----> SESSION_REGISTRY
    SESSION_REGISTRY ----> SESSION
    SESSION ----> ENGINE

    style CLIENT color:#FFFFFF, fill:#575279, stroke:#575279
    style SESSION_CONTROLLER color:#FFFFFF, fill:#907aa9, stroke:#907aa9
    style STATIC_DATA_CONTROLLER color:#FFFFFF, fill:#907aa9, stroke:#907aa9
    style STREAM_CONTROLLER color:#FFFFFF, fill:#907aa9, stroke:#907aa9
    style SESSION_REGISTRY color:#FFFFFF, fill:#56949f, stroke:#56949f
    style SESSION color:#FFFFFF, fill:#56949f, stroke:#56949f
    style ENGINE color:#FFFFFF, fill:#b4637a, stroke:#b4637a
```

## Classes

| Class                  | Endpoints                                                                                                                    | What it does                                                                                                                                                |
| ---------------------- | ---------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `SessionController`    | `GET /health`, `GET /datastores`, `POST /datastores/{name}`, `DELETE /datastores/{name}`                                     | Health check; lists, creates and deletes datastores. Each datastore is a `Session` with its own engine.                                                     |
| `StaticDataController` | `POST /datastores/{name}/content`, `POST /datastores/{name}/sparql`                                                          | Loads Turtle into the engine's store and runs one-shot SPARQL SELECT queries on it, as SPARQL JSON or TSV depending on `Accept`.                            |
| `StreamController`     | `POST /datastores/{name}/queries`, `POST /datastores/{name}/streams/{stream}`, `GET /datastores/{name}/queries/{id}/results` | Registers continuous CQELS-QL queries, pushes stream elements with their `eventTime`, and returns the results collected since the last call as SPARQL JSON. |

## Conventions

- **Lookup:** every endpoint below `/datastores/{name}` gets the session from `SessionRegistry` and returns `404` if it does not exist.
- **Errors:** invalid input (Turtle, SPARQL, CQELS-QL, `eventTime`) returns `400` with the reason in the body.
- **No logic here:** ordering of stream elements, result buffering and the engine lifecycle live in the `service` package. The controllers only parse, delegate and format.

## Typical call order

1. `POST /datastores/{name}` creates the session.
2. `POST .../content` loads static data, if the queries need it.
3. `POST .../queries` registers the continuous queries, before the first stream element.
4. `POST .../streams/{stream}?eventTime=...` pushes one element per signal.
5. `GET .../queries/{id}/results` fetches the new results, for example after each push.
6. `DELETE /datastores/{name}` stops the engine.
