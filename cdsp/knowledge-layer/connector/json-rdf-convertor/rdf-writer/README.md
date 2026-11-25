# RDF Triple Assembler

RDF Triple Assembler transforms structured data into RDF triples using the [RDFAdapter](/cdsp/knowledge-layer/symbolic-reasoner/adapters/rdfox/README.md) and `TripleWriter` components. It enables developers to convert messages into RDF (Resource Description Framework) triples for semantic data storage and query purposes.

## Components

### TripleAssembler

`TripleAssembler` processes data messages, extracting nodes, querying RDF properties, and calling [`TripleWriter`](#rdf-triple-writer) to generate triples. The main function is `transformMessageToRDFTriple`, which converts each node in the message into RDF triples and stores them in the configured output file. When the reasoner settings contain `frames`, it hands the resolved nodes to the [`FrameBuilder`](#frame-builder) and stores the frames it returns instead.

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

### Frame Builder

`FrameBuilder` builds frames when the reasoner settings contain `frames`. Which inference engines support frames is checked when the model configuration is read, see the [model configuration](../../../symbolic-reasoner/examples/use-case/README.md). It keeps the latest value of each input signal and builds one frame per moment: a `sosa:ObservationCollection` with one member observation (`sosa:hasMember`) per signal, written through the `TripleWriter`.

- Values are kept per vehicle identifier (VIN from the message header), so a frame only contains the values of its own vehicle.
- An older value never replaces a newer one.
- The signals of one moment can arrive in several messages (one per signal). Messages with the same latest timestamp belong to one moment. Its frame is built when the first message of a later moment arrives, so the frame of the latest moment waits for newer data.
- A value older than the maximum age before the frame time is left out (setting `frames.max_value_age_ms`, default 1 s).
- Latitude and longitude are only taken together, with their NTM values.

Each member has its own feature of interest, property, value with datatype and `sosa:phenomenonTime`, like an observation without frames. Its IRI carries the VIN and the frame timestamp, for example `car:fm_Speed_VIN123_20261116100002200000000`.

# Testing

An Integration [test](../tests/) is provided to ensure functionality. Tests cover the main components (`TripleAssembler`, `TripleWriter`, `FrameBuilder`) to verify message transformation, RDF generation, and error handling.
