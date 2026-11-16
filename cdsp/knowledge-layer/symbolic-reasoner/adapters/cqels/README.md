# CQELS Integration

This folder contains the necessary files to interact with the CQELS reasoner service using the [CQELS adapter](./src/README.md). CQELS is a stream reasoner: continuous queries in CQELS-QL run over time windows of the observations.

## How to Use CQELS

For instructions on how to start the CQELS service required for this project, see [this guide](../../../../reasoner-services/cqels/README.md).

### RDF Triple Assembler

See how the observations reach the CQELS service using the [RDF assembler](/cdsp/knowledge-layer/connector/json-rdf-convertor/rdf-writer/README.md). Each generated set of triples is pushed as one stream element, with the observation time as event time.
