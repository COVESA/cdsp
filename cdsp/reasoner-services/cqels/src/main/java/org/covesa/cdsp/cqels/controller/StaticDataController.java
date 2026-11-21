package org.covesa.cdsp.cqels.controller;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.StringReader;
import org.cqels.engine.CQELSEngine;
import org.covesa.cdsp.cqels.service.Session;
import org.covesa.cdsp.cqels.service.SessionRegistry;
import org.eclipse.rdf4j.common.exception.RDF4JException;
import org.eclipse.rdf4j.query.resultio.TupleQueryResultWriter;
import org.eclipse.rdf4j.query.resultio.sparqljson.SPARQLResultsJSONWriter;
import org.eclipse.rdf4j.query.resultio.text.tsv.SPARQLResultsTSVWriter;
import org.eclipse.rdf4j.repository.RepositoryConnection;
import org.eclipse.rdf4j.rio.RDFFormat;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestHeader;
import org.springframework.web.bind.annotation.RestController;

/**
 * Static data of a session (ontologies, SHACL shapes) in the engine's own
 * store, and one-shot SPARQL SELECT queries on it.
 */
@RestController
public class StaticDataController {

    private static final String SPARQL_JSON = "application/sparql-results+json";
    private static final String TSV = "text/tab-separated-values";

    private final SessionRegistry sessions;

    /** @param sessions the sessions shared by all endpoints */
    public StaticDataController(SessionRegistry sessions) {
        this.sessions = sessions;
    }

    /**
     * Loads static data (for example SHACL shapes) into the session's store.
     *
     * @param name   the session (datastore) name
     * @param turtle the data in Turtle
     * @return 200 if loaded, 400 on invalid Turtle, 404 if the session does not
     *         exist
     */
    @PostMapping("/datastores/{name}/content")
    public ResponseEntity<String> load(@PathVariable String name, @RequestBody String turtle) {
        CQELSEngine engine = sessions.get(name).map(Session::engine).orElse(null);
        if (engine == null) {
            return ResponseEntity.notFound().build();
        }
        try (RepositoryConnection conn = engine.getRepository().getConnection()) {
            conn.add(new StringReader(turtle), "", RDFFormat.TURTLE);
            return ResponseEntity.ok("Content imported successfully.");
        } catch (RDF4JException | IOException e) {
            return ResponseEntity.badRequest().body("Invalid Turtle: " + e.getMessage());
        }
    }

    /**
     * Runs a one-shot SELECT on the session's store.
     *
     * @param name   the session (datastore) name
     * @param sparql the SELECT query
     * @param accept SPARQL JSON if it contains "json", otherwise TSV, like the
     *               RDF4J service
     * @return 200 with the results, 400 on an invalid query, 404 if the session
     *         does not exist
     */
    @PostMapping("/datastores/{name}/sparql")
    public ResponseEntity<String> query(@PathVariable String name, @RequestBody String sparql,
            @RequestHeader(value = "Accept", defaultValue = "*/*") String accept) {
        CQELSEngine engine = sessions.get(name).map(Session::engine).orElse(null);
        if (engine == null) {
            return ResponseEntity.notFound().build();
        }
        boolean json = accept.contains("json");
        ByteArrayOutputStream out = new ByteArrayOutputStream();
        TupleQueryResultWriter writer = json ? new SPARQLResultsJSONWriter(out) : new SPARQLResultsTSVWriter(out);
        try (RepositoryConnection conn = engine.getRepository().getConnection()) {
            conn.prepareTupleQuery(sparql).evaluate(writer);
        } catch (RDF4JException e) {
            return ResponseEntity.badRequest().body("Invalid SPARQL query: " + e.getMessage());
        }
        return ResponseEntity.ok().header("Content-Type", json ? SPARQL_JSON : TSV).body(out.toString());
    }
}
