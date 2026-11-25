# CQELS Integration

This folder contains the necessary files to interact with the CQELS reasoner service using the [CQELS adapter](./src/README.md). CQELS is a stream reasoner: continuous queries in CQELS-QL run over time windows of the observations.

## How to Use CQELS

For instructions on how to start the CQELS service required for this project, see [this guide](../../../../reasoner-services/cqels/README.md).

### RDF Triple Assembler

See how the observations reach the CQELS service using the [RDF assembler](/cdsp/knowledge-layer/connector/json-rdf-convertor/rdf-writer/README.md). Each generated set of triples is pushed as one stream element, with the observation time as event time. With `"frames"` in the reasoner settings, each element is one frame: the latest values of all signals of one vehicle at one moment. A frame is pushed when the first data of the next moment arrives. See the [model configuration](../../examples/use-case/README.md#reasoner-settings).

## Differences to RDFox and RDF4J

- **Continuous queries instead of rules.** The `.cqels` files listed under `rules` are continuous queries. They do not add triples to the datastore; they only return result rows. The same file in the `queries.output` folder reads these rows.
- **Only new results.** Each read returns the rows since the previous read. RDFox and RDF4J return the results that match the data in the datastore at the time of the query.
- **Time windows.** A query only sees the stream elements in its window (for example `[RANGE 3s]`), ordered by the time of the observation. Observations outside the window are not seen by the query.
- **Results only with new data.** A window is evaluated when a newer element arrives. When the vehicle stops sending, the last window gives no result.
- **Frames for joins.** A query that joins several signals by time with one observation per signal reaches the join limit of the engine. Such queries need `"frames"`, see the [CQELS service README](../../../../reasoner-services/cqels/README.md#limitations).
- **Number types.** Numbers come back as Java types, for example `xsd:double` for an `xsd:float` input, so their text form can differ from RDFox and RDF4J.
