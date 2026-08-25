# S2DM Config Generator

Generates the two config files the Information Layer (IL) and Knowledge Layer (KL) read at
startup, from a single GraphQL schema (`cdsp-custom.graphql`), using
[S2DM](https://github.com/COVESA/s2dm):

| Generated file                                  | Consumed by             |
| ----------------------------------------------- | ----------------------- |
| `KL-config/rdfox_model/shacl/vehicle_shacl.ttl` | Knowledge Layer (RDFox) |
| `KL-config/rdf4j_model/shacl/vehicle_shacl.ttl` | Knowledge Layer (RDF4J) |
| `IL-config/schema-files/vss_data_points.yaml`   | Information Layer       |

Both SHACL copies are always written identically, regardless of which reasoner profile you use.

This replaces hand-maintaining those files independently. Change the schema once, regenerate, and both layers stay in sync.

## Running it standalone

```bash
cd examples/knowledgelayer-hello-world/s2dm
uv sync
uv run s2dmc
```

This writes directly onto the three paths above (relative to this folder's parent), so no
further copy step is needed. Start the stack with `docker compose up` right after and IL/KL
will read the freshly generated files.

Run the self-check anytime with:

```bash
uv run pytest
```

> [!NOTE]
> You can also run a full end-to-end test, feeding real signal data through IL/KL and confirming KL's
> reasoning rules produce a result that lives in `../test/feed_test_data.py`.

## CLI parameters

| Flag              | Default                                                  | Notes                                                                                                                    |
| ----------------- | -------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------ |
| `--schema`        | bundled `cdsp-custom.graphql`                            | any compatible SDL accepted                                                                                              |
| `--naming-config` | bundled `naming.yaml`                                    | passed as-is to S2DM                                                                                                     |
| `--il-out`        | `../IL-config/schema-files/vss_data_points.yaml`         |                                                                                                                          |
| `--kl-rdfox-out`  | `../KL-config/rdfox_model/shacl/vehicle_shacl.ttl`       |                                                                                                                          |
| `--kl-rdf4j-out`  | `../KL-config/rdf4j_model/shacl/vehicle_shacl.ttl`       |                                                                                                                          |

S2DM itself always runs at the version pinned in `pyproject.toml`.

## Docker Compose (automatic)

`docker-compose.yml` wires this in as the `s2dm-config-gen` service, a one-shot init container
(same pattern as `rdfox-init`) that runs before `information-layer` and both `knowledge-layer-*`
services, which wait on `service_completed_successfully`. Nothing new to run, `docker compose up`
already regenerates both config files before anything else starts.

> [!IMPORTANT]
> `s2dm-config-gen`'s image bakes in `cdsp-custom.graphql` and `naming.yaml` at _build_
> time. `docker compose up` only builds an image if one doesn't already exist locally. It won't
> notice you edited the schema. After changing `cdsp-custom.graphql` or `naming.yaml`, rebuild
> explicitly:
>
> ```bash
> docker compose up --build
> # or: docker compose build s2dm-config-gen
> ```

If the schema is invalid, `s2dm-config-gen` exits non-zero and Compose correctly refuses to start
IL/KL at all, rather than running them against stale or half-written config.

## Post-processing: what gets fixed and why

S2DM's raw export output has a few gaps that would otherwise break IL or KL. `s2dmc` (`src/s2dmc/main.py`)
corrects them after export:

- **Acronym casing** (`Ai`→`AI`, `Vin`→`VIN`, `PwfStatus`→`PWFStatus`): S2DM's PascalCase
  converter treats acronyms as regular words. Fixed in both the vspec path segments and the SHACL
  `sh:name` values.
- **Integer width**: `cdsp-custom.graphql` uses `Int8`/`UInt8`/`Int16`/`UInt16` scalars (instead
  of plain `Int`) for fields IL requires a specific width for. IL's IoTDB handler only accepts
  `boolean/string/float/double/int8/int16/uint8/uint16`, and a plain `Int` (→ vspec `int32`)
  crashes it outright at startup. S2DM's vspec exporter maps those scalars correctly, but its
  SHACL exporter doesn't yet map them to an XSD type (falls back to `xsd:string`). This script
  restores `sh:datatype xsd:integer` for those fields.

Both of these are workarounds for confirmed upstream S2DM issues, not permanent design choices.
They're tracked with the S2DM maintainer; once fixed there, the corresponding functions in
`src/s2dmc/main.py` (marked with a comment naming exactly what to remove) can go away.

## Known gaps (not fixed but confirmed non-blocking)

| Gap                                                                       | Impact                                                                                      |
| ------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------- |
| `xsd:float` instead of `xsd:double` for lat/lon                           | Precision difference only; no `Double` type in GraphQL                                      |
| `xsd:integer` instead of `xsd:int` for other integer fields               | Semantically equivalent; both reasoners accept it                                           |
| Shape IRI `val:Vehicle` instead of `val:VehicleShape`                     | Cosmetic; not used for validation                                                           |
| `car:hasChassis`/`car:hasPowertrain` etc. instead of a flat `car:hasPart` | Not a blocker: KL reads relationship predicates dynamically from the SHACL                  |
| vspec leaf nodes missing `type: sensor`/`attribute`                       | Not a blocker: IL classifies leaf nodes by the presence of `datatype:`, never reads `type:` |
