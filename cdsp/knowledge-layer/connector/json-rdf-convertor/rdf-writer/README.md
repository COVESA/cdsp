# RDF Triple Assembler

RDF Triple Assembler transforms structured data into RDF triples using the [RDFAdapter](/cdsp/knowledge-layer/symbolic-reasoner/rdfox/README.md) and `TripleWriter` components. It enables developers to convert messages into RDF (Resource Description Framework) triples for semantic data storage and query purposes.

## Components

### TripleAssembler

`TripleAssembler` processes data messages, extracting nodes, querying RDF properties, and calling [`TripleWriter`](#rdf-triple-writer) to generate triples. The main function is `transformMessageToRDFTriple`, which converts each node in the message into RDF triples and stores them in the configured output file.

### RDF Triple Writer

`TripleWriter` creates and manages RDF triples using the [Serd library](https://drobilla.net/software/serd.html). It supports adding object and data triples with prefixes, generating the RDF output in any of this formats:

- [Turtle](https://www.w3.org/TR/turtle/) is often used for readability and compactness, making it ideal for documents that will be read and edited by humans.
- [N-Triples](https://www.w3.org/TR/n-triples/) is suitable for large datasets or streaming because of its simplicity and line-based structure, which is easy to parse.
- [N-Quads](https://www.w3.org/TR/n-quads/) is useful when you need to include additional contextual information (e.g., the source or graph of a triple).
- [TriG](https://www.w3.org/TR/trig/) provides a structured way to group related triples, making it useful for complex datasets with multiple contexts or named graphs.

**Example Usage**
```cpp
TripleWriter writer;
writer.setTripleIdentifier("ExampleID");
writer.addRDFObjectToTriple("prefix car: <http://example.com/car#>", std::make_tuple("car:Vehicle", "car:hasPart", "car:Engine"));
std::string rdf_output = writer.generateTripleOutput(ReasonerSyntaxType::TURTLE);
```

# Testing

An Integration [test](../tests/) is provided to ensure functionality. Tests cover the main components (`TripleAssembler`, `TripleWriter`) to verify message transformation, RDF generation, and error handling.