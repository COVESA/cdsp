
# RDFox Integration

This folder contains the necessary files to interact with RDFox using the [RDFox adapter](./src/README.md), a high-performance knowledge graph and reasoning engine used in this project.

## How to Use RDFox

For instructions on how to start the RDFox service required for this project, see [this guide](/docker/README.md#rdfox-restful-api).

### RDF Triple Assembler

See how to interact with the RDFox server using the [RDF assembler](/cdsp/knowledge-layer/connector/json-rdf-convertor/rdf-writer/README.md). This module is designed to transform structured data messages into RDF triples, providing a streamlined way to produce linked data for knowledge graphs, ontologies, or semantic applications. Using TripleAssembler, users can process messages and generate triples that can be serialized in various RDF formats such as Turtle, N-Triples, or TriG.

### RDFox Service Test

This project includes a small test C++ application to verify that the RDFox service has been configured and started correctly. After compiling the project, you should be able to run the application from [`./tests/rdfox-service-test/rdfox_test_main.cpp`](./tests/rdfox-service-test/rdfox_test_main.cpp). The RDFox Test executable will be generated in the `/cdsp/knowledge-layer/build/bin/tests/` directory. You can run it with the following command:

```bash
$ ./rdfox_test

# Data store list:
# ?Name	?UniqueID	?Persistent	?Online	?Parameters

# Data store 'family' does not exist.
# Creating a new data store 'family'.
# Data store created.
# Facts added.
# SPARQL query result:
# ?p	?n
# <https://oxfordsemantic.tech/RDFox/getting-started/peter>	"Peter"
# <https://oxfordsemantic.tech/RDFox/getting-started/stewie> "Steve"
# <https://oxfordsemantic.tech/RDFox/getting-started/chris>	"Chris"
# <https://oxfordsemantic.tech/RDFox/getting-started/meg>	"Meg"
# <https://oxfordsemantic.tech/RDFox/getting-started/lois>	"Lois"

# Data store 'family' deleted successfully.
```