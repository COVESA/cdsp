# RDFoxAdapter

`RDFoxAdapter` is responsible for communicating with the RDFox server over HTTP. It checks if the datastore is active, loads RDF data, and queries the RDF data using [SPARQL](https://www.w3.org/TR/sparql11-query/).

> [!NOTE] Data store
> When the RDFoxAdapter initializes creates (if is does not exists) a datastore called `vehicle_ds` in the RDFox server.
>
> ## Features

- **Initialize a Datastore:** Checks if a specified datastore exists on the RDFox server. If not, it creates the datastore.
- **Load Data:** Loads Turtle data into a specified datastore.
- **Query Data:** Executes SPARQL queries against the RDFox datastore and retrieves results.
- **Delete Datastore:** Removes a specified datastore from the RDFox server.

**Example Usage**

```cpp
RDFoxAdapter adapter("<rdf_host>", "<port>", "<auth>", "<datastore>");
if (adapter.checkDataStore()) {
    adapter.loadData("<your_ttl_data>");
    std::string result = adapter.queryData("SELECT ?s ?p ?o WHERE { ?s ?p ?o }");
}
```

# Testing

Unit and Integration [tests](../tests/) are provided to ensure functionality. Tests cover the main components (`RDFoxAdapter`) to verify RDF data transfer, and error handling.
