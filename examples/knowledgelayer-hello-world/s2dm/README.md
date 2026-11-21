# S2DM Config Generator

Generates the two config files the Information Layer (IL) and Knowledge Layer (KL) read at
startup, from a single GraphQL schema (`cdsp-custom.graphql`), using
[S2DM](https://github.com/COVESA/s2dm):

| Generated file                                | Consumed by                            |
| --------------------------------------------- | -------------------------------------- |
| `KL-config/shared/shacl/vehicle_shacl.ttl`    | Knowledge Layer (every reasoner)       |
| `IL-config/schema-files/vss_data_points.yaml` | Information Layer                      |

The SHACL file is written once into `KL-config/shared/`, which every reasoner configuration (`rdfox_model`, `rdf4j_model`) reads.

This replaces hand-maintaining those files independently. Change the schema once, regenerate, and both layers stay in sync.

## Running it standalone

```bash
cd examples/knowledgelayer-hello-world/s2dm
uv sync
uv run s2dmc
```

This writes directly onto the two paths above (relative to this folder's parent), so no
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
| `--kl-out`        | `../KL-config/shared/shacl/vehicle_shacl.ttl`            | shared by all reasoners                                                                                                  |

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

## Naming exceptions and integer types

`naming.yaml`'s `exceptions` key lists literal name overrides applied before S2DM's case
conversion, for any identifier that must keep an exact casing instead of being reshaped into
PascalCase/camelCase, such as acronyms (`AI`, `VIN`, `PWFStatus`). Add an entry there whenever
the schema introduces a new one that needs preserving.

`cdsp-custom.graphql` uses the `Int8`/`UInt8`/`Int16`/`UInt16` scalars (instead of plain `Int`)
for fields IL requires a specific width for, since IL's IoTDB handler only accepts
`boolean/string/float/double/int8/int16/uint8/uint16`. S2DM maps each of these scalars to its
precise XSD type (`xsd:byte`, `xsd:unsignedByte`, `xsd:short`, `xsd:unsignedShort`) in the
generated SHACL shapes.

## Known gaps (not fixed but confirmed non-blocking)

| Gap                                                                       | Impact                                                                                      |
| ------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------- |
| `xsd:float` instead of `xsd:double` for lat/lon                           | Precision difference only; no `Double` type in GraphQL                                      |
| `xsd:integer` instead of `xsd:int` for other integer fields               | Semantically equivalent; both reasoners accept it                                           |
| Shape IRI `val:Vehicle` instead of `val:VehicleShape`                     | Cosmetic; not used for validation                                                           |
| `car:hasChassis`/`car:hasPowertrain` etc. instead of a flat `car:hasPart` | Not a blocker: KL reads relationship predicates dynamically from the SHACL                  |
| vspec leaf nodes missing `type: sensor`/`attribute`                       | Not a blocker: IL classifies leaf nodes by the presence of `datatype:`, never reads `type:` |
