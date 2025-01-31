# Serd Installation Test

This simple application explains how to work with the Serd library. Serd is a lightweight C library for parsing and serializing RDF data. For more information, refer to the [Serd](https://drobilla.net/software/serd.html) and [Serd C API](https://drobilla.gitlab.io/serd/doc/html/api/serd.html) documentation.

## Purpose

### Serd triple writer `serd_triple_writer_test.cpp` using `Serd C API`

This C++ program demonstrates how to use the Serd library to create and write RDF triples in Turtle format to standard output. The example defines RDF resources, namespaces, and data literals, and writes statements (triples) describing a Vehicle entity and its attributes.

Key sections of the code:

1. **Setting Up the Serd Environment:** Initializes a Serd environment, defines a base URI and creates the Serd writer configured to output in Turtle format.
2. **Namespace Declaration:** Defines different namespaces to be used.
3. **Creating RDF Nodes:** Defines nodes for subject, predicate, and object URIs, as well as literal values.
4. **Writing RDF Statements:** Writes the RDF triples.
5. **Cleaning Up:** Frees allocated resources and finalizes the writer.

Output

The program outputs RDF triples in Turtle format to standard output. Example output:

```turtle
@prefix ex: <http://example.org/> .
@prefix xsd: <http://www.w3.org/2001/XMLSchema#> .

ex:Vehicle123 a ex:Vehicle ;
    ex:hasSpeed "120"^^xsd:int .
```

After the project is compiled, it can be run with:

```bash
./serd_triple_writer_example
```