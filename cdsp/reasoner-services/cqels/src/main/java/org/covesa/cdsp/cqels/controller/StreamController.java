package org.covesa.cdsp.cqels.controller;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.StringReader;
import java.time.Instant;
import java.time.OffsetDateTime;
import java.time.format.DateTimeParseException;
import java.util.Map;
import java.util.Optional;
import org.covesa.cdsp.cqels.service.Session;
import org.covesa.cdsp.cqels.service.SessionRegistry;
import org.eclipse.rdf4j.common.exception.RDF4JException;
import org.eclipse.rdf4j.model.Model;
import org.eclipse.rdf4j.model.Value;
import org.eclipse.rdf4j.model.util.Values;
import org.eclipse.rdf4j.query.impl.MapBindingSet;
import org.eclipse.rdf4j.query.resultio.sparqljson.SPARQLResultsJSONWriter;
import org.eclipse.rdf4j.rio.RDFFormat;
import org.eclipse.rdf4j.rio.Rio;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

/**
 * Continuous queries of a session, the stream elements they run on, and their
 * results.
 */
@RestController
public class StreamController {

    private static final Logger logger = LoggerFactory.getLogger(StreamController.class);
    private final SessionRegistry sessions;

    /** @param sessions the sessions shared by all endpoints */
    public StreamController(SessionRegistry sessions) {
        this.sessions = sessions;
    }

    /**
     * Registers a continuous query. Register the queries before the first stream
     * element.
     *
     * @param name    the session (datastore) name
     * @param cqelsQl the query in CQELS-QL
     * @return 200 with the query id, 400 with the engine's message if the query is
     *         rejected, 404 if the session does not exist
     */
    @PostMapping("/datastores/{name}/queries")
    public ResponseEntity<String> register(@PathVariable String name, @RequestBody String cqelsQl) {
        Optional<Session> session = sessions.get(name);
        if (session.isEmpty()) {
            return ResponseEntity.notFound().build();
        }
        try {
            String id = session.get().registerQuery(cqelsQl);
            logger.info("Registered query {} in session {}", id, name);
            return ResponseEntity.ok(id);
        } catch (RuntimeException e) {
            return ResponseEntity.badRequest().body("Invalid CQELS-QL query: " + e.getMessage());
        }
    }

    /**
     * Pushes one stream element: all triples of the request, observed at eventTime.
     *
     * @param name      the session (datastore) name
     * @param stream    the stream name used in the queries
     * @param eventTime the observation time in ISO-8601 with offset, for example
     *                  2026-10-30T10:15:30.250Z
     * @param turtle    the triples in Turtle
     * @return 200 if pushed, 400 on invalid Turtle or eventTime, 404 if the session
     *         does not exist
     */
    @PostMapping("/datastores/{name}/streams/{stream}")
    public ResponseEntity<String> push(@PathVariable String name, @PathVariable String stream,
            @RequestParam String eventTime, @RequestBody String turtle) {
        Optional<Session> session = sessions.get(name);
        if (session.isEmpty()) {
            return ResponseEntity.notFound().build();
        }
        Instant time;
        Model statements;
        try {
            time = OffsetDateTime.parse(eventTime).toInstant();
        } catch (DateTimeParseException e) {
            return ResponseEntity.badRequest().body("Invalid eventTime: " + e.getMessage());
        }
        try {
            statements = Rio.parse(new StringReader(turtle), "", RDFFormat.TURTLE);
        } catch (RDF4JException | IOException e) {
            return ResponseEntity.badRequest().body("Invalid Turtle: " + e.getMessage());
        }
        session.get().push(stream, statements, time);
        return ResponseEntity.ok("Element pushed.");
    }

    /**
     * Returns the results of a continuous query since the last call, so each result
     * is returned only once.
     *
     * @param name the session (datastore) name
     * @param id   the query id returned at registration
     * @return 200 with the results as SPARQL JSON, 404 if the session or query does
     *         not exist
     */
    @GetMapping(value = "/datastores/{name}/queries/{id}/results", produces = "application/sparql-results+json")
    public ResponseEntity<String> results(@PathVariable String name, @PathVariable String id) {
        Optional<Session.Results> results = sessions.get(name).flatMap(s -> s.drainResults(id));
        if (results.isEmpty()) {
            return ResponseEntity.notFound().build();
        }
        return ResponseEntity.ok(toSparqlJson(results.get()));
    }

    /**
     * Writes query results as SPARQL JSON.
     *
     * @param results the query's variables and result rows
     * @return the SPARQL JSON document
     */
    private static String toSparqlJson(Session.Results results) {
        ByteArrayOutputStream out = new ByteArrayOutputStream();
        SPARQLResultsJSONWriter writer = new SPARQLResultsJSONWriter(out);
        writer.startQueryResult(results.variables());
        for (Map<String, Object> row : results.rows()) {
            MapBindingSet bindings = new MapBindingSet();
            row.forEach((var, value) -> {
                if (value != null) {
                    bindings.addBinding(var, toValue(value));
                }
            });
            writer.handleSolution(bindings);
        }
        writer.endQueryResult();
        return out.toString();
    }

    /**
     * Turns a result value into an RDF value; plain Java values become typed
     * literals.
     *
     * @param value an RDF value or a Java value such as a number, boolean or string
     * @return the RDF value
     */
    private static Value toValue(Object value) {
        return value instanceof Value rdf ? rdf : Values.literal(value);
    }
}
