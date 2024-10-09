
# RDFox Integration

This folder contains the necessary files to interact with RDFox, a high-performance knowledge graph and reasoning engine used in this project.

## How to Use RDFox

For instructions on how to start the RDFox service required for this project, see [this guide](../../../../docker/README.md#rdfox-restful-api).

### Getting Started

This project includes a small C++ application to verify that the RDFox service has been configured and started correctly. After compiling the project, you should be able to run the application from [`./rdfox-install-test/rdfox_test_main.cpp`](./rdfox-install-test/rdfox_test_main.cpp). The RDFox Test executable will be generated in the `/cdsp/knowledge-layer/build/bin/` directory. You can run it with the following command:

```bash
$ ./websocket_client

# Data store list:
# ?Name	?UniqueID	?Persistent	?Online	?Parameters

# Data store 'family' does not exist.
# Creating a new data store 'family'.
# Data store created.
# Facts added.
# SPARQL query result:
# ?p	?n
# <https://oxfordsemantic.tech/RDFox/getting-started/peter>	"Peter"
# <https://oxfordsemantic.tech/RDFox/getting-started/stewie>	"Stewie"
# <https://oxfordsemantic.tech/RDFox/getting-started/chris>	"Chris"
# <https://oxfordsemantic.tech/RDFox/getting-started/meg>	"Meg"
# <https://oxfordsemantic.tech/RDFox/getting-started/lois>	"Lois"

# Data store 'family' deleted successfully.
```