# Source code

The Java source of the CQELS reasoner service, a Spring Boot application. How to build, run and configure the service is in the [service README](../../../../../../../README.md).

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
    APPLICATION(CqelsReasonerApplication)
    CONTROLLER(controller)
    SERVICE(service)
    ENGINE(cqels-engine)

    APPLICATION -- starts --> CONTROLLER
    CONTROLLER -- calls --> SERVICE
    SERVICE -- calls --> ENGINE

    style APPLICATION color:#FFFFFF, fill:#575279, stroke:#575279
    style CONTROLLER color:#FFFFFF, fill:#907aa9, stroke:#907aa9
    style SERVICE color:#FFFFFF, fill:#56949f, stroke:#56949f
    style ENGINE color:#FFFFFF, fill:#b4637a, stroke:#b4637a
```

## Contents

| Path                               | What it holds                                                                                                   |
| ---------------------------------- | --------------------------------------------------------------------------------------------------------------- |
| `CqelsReasonerApplication.java`    | Entry point. Starts Spring Boot and enables scheduling, which the reorder buffer uses to release held elements. |
| [controller](controller/README.md) | The REST endpoints: parse requests, call the service package, format responses.                                 |
| [service](service/README.md)       | Sessions, the CQELS engines, and the reorder and result buffers.                                                |

The settings and their environment variables (`SERVER_PORT`, `CQELS_RESULTS_BUFFER_SIZE`, `CQELS_REORDER_TOLERANCE`) are in `src/main/resources/application.properties`.

## Tests

The tests in `src/test/java/org/covesa/cdsp/cqels` follow the same structure. Their names end in `UnitTest` (one class, no Spring) or `IntegrationTest` (the whole application through its REST endpoints). Run them with `mvn test`.
