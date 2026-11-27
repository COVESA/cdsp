package org.covesa.cdsp.rdf4j.service;

import java.nio.charset.StandardCharsets;
import java.io.StringReader;
import java.io.ByteArrayOutputStream;
import java.util.List;
import java.util.ArrayList;
import java.util.Collections;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.ArrayBlockingQueue;
import org.eclipse.rdf4j.repository.Repository;
import org.eclipse.rdf4j.repository.RepositoryConnection;
import org.eclipse.rdf4j.query.resultio.sparqljson.SPARQLResultsJSONWriter;
import org.eclipse.rdf4j.query.resultio.text.tsv.SPARQLResultsTSVWriter;
import org.eclipse.rdf4j.query.TupleQuery;
import org.eclipse.rdf4j.query.GraphQuery;
import org.eclipse.rdf4j.query.GraphQueryResult;
import org.eclipse.rdf4j.rio.RDFFormat;
import org.eclipse.rdf4j.model.IRI;
import org.eclipse.rdf4j.model.Statement;
import org.eclipse.rdf4j.model.Value;
import org.eclipse.rdf4j.model.Literal;
import org.eclipse.rdf4j.repository.RepositoryResult;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Service;
import jakarta.annotation.PreDestroy;

@Service
public class ReasonerService {

    private static final Logger logger = LoggerFactory.getLogger(ReasonerService.class);
    private final Repository repository;
    private static final String SHACL_SHAPE_GRAPH_URI = "http://rdf4j.org/schema/rdf4j#SHACLShapeGraph";
    private final RepositoryConnection writeConnection;
    private final List<GraphQuery> precompiledRules = new ArrayList<>();
    private final List<String> cachedRuleStrings = new ArrayList<>();

    private final ExecutorService reasoningExecutor = new ThreadPoolExecutor(
            1, 1, // 1. Thread Pool Size: Only 1 thread to process reasoning tasks
            0L, TimeUnit.MILLISECONDS, // 2. Keep-Alive Time: Not applicable since corePoolSize == maximumPoolSize
            new ArrayBlockingQueue<>(5000), // 3. The Queue: 5000 tasks max
            new ThreadPoolExecutor.CallerRunsPolicy()); // 4. The Rejection Policy: If queue is full, run in main thread

    public ReasonerService(Repository repository) {
        this.repository = repository;
        this.writeConnection = repository.getConnection();
    }

    // Extracts and compiles sh:rule/sh:construct queries from the shape graph
    private void cacheRules(RepositoryConnection conn) {
        IRI shapeGraphIRI = conn.getValueFactory().createIRI(SHACL_SHAPE_GRAPH_URI);
        String SHACL_NS = "http://www.w3.org/ns/shacl#";
        IRI shRule = conn.getValueFactory().createIRI(SHACL_NS, "rule");
        IRI shConstruct = conn.getValueFactory().createIRI(SHACL_NS, "construct");
        IRI shOrder = conn.getValueFactory().createIRI(SHACL_NS, "order");

        List<Rule> rules = new ArrayList<>();
        try (RepositoryResult<Statement> result = conn
                .getStatements(null, shRule, null, shapeGraphIRI)) {
            for (Statement st : result) {
                org.eclipse.rdf4j.model.Resource ruleNode = (org.eclipse.rdf4j.model.Resource) st.getObject();

                // Retrieve the sh:order value for sorting
                double order = 0.0;
                try (RepositoryResult<Statement> orderSts = conn
                        .getStatements(ruleNode, shOrder, null, shapeGraphIRI)) {
                    if (orderSts.hasNext()) {
                        Value o = orderSts.next().getObject();
                        if (o instanceof Literal) {
                            order = ((Literal) o).doubleValue();
                        }
                    }
                }

                // Retrieve the sh:construct query string
                try (RepositoryResult<Statement> constructSts = conn
                        .getStatements(ruleNode, shConstruct, null, shapeGraphIRI)) {
                    for (Statement constructSt : constructSts) {
                        rules.add(new Rule(constructSt.getObject().stringValue(), order));
                    }
                }
            }
        }
        Collections.sort(rules);
        cachedRuleStrings.clear();
        for (Rule r : rules) {
            cachedRuleStrings.add(r.queryString);
        }
        logger.info("Cached {} rules (sorted by sh:order).", cachedRuleStrings.size());
    }

    // Replaces all rules with the provided Turtle content
    public void resetRules(String newRuleData) {
        synchronized (writeConnection) {
            try {
                // Clear the existing shape graph
                IRI shapeGraphIRI = writeConnection.getValueFactory().createIRI(SHACL_SHAPE_GRAPH_URI);
                writeConnection.begin();
                writeConnection.clear(shapeGraphIRI);
                precompiledRules.clear();
                cachedRuleStrings.clear();

                // Add new rules to the shape graph
                writeConnection.add(new StringReader(newRuleData), "", RDFFormat.TURTLE, shapeGraphIRI);
                cacheRules(writeConnection);
                for (String rule : cachedRuleStrings) {
                    precompiledRules.add(writeConnection.prepareGraphQuery(rule));
                }
                writeConnection.commit();
                logger.info("Rules reset successfully ({} rules compiled).", precompiledRules.size());
            } catch (Exception e) {
                if (writeConnection.isActive()) {
                    writeConnection.rollback();
                }
                logger.error("Error during rule reset", e);
                throw new RuntimeException("Failed to reset rules", e);
            }
        }
    }

    // Appends rules to existing ones
    public void addRules(String newRuleData) {
        synchronized (writeConnection) {
            try {
                IRI shapeGraphIRI = writeConnection.getValueFactory().createIRI(SHACL_SHAPE_GRAPH_URI);
                writeConnection.begin();
                writeConnection.add(new StringReader(newRuleData), "", RDFFormat.TURTLE, shapeGraphIRI);
                precompiledRules.clear();
                cachedRuleStrings.clear();
                cacheRules(writeConnection);
                for (String rule : cachedRuleStrings) {
                    precompiledRules.add(writeConnection.prepareGraphQuery(rule));
                }
                writeConnection.commit();
                logger.info("Rules appended successfully ({} rules compiled).", precompiledRules.size());
            } catch (Exception e) {
                if (writeConnection.isActive()) {
                    writeConnection.rollback();
                }
                logger.error("Error during rule append", e);
                throw new RuntimeException("Failed to append rules", e);
            }
        }
    }

    // Ingests RDF data and triggers rule execution asynchronously
    public void addData(String rdfData) {
        reasoningExecutor.submit(() -> {
            synchronized (writeConnection) {
                try {
                    writeConnection.begin();
                    writeConnection.add(new StringReader(rdfData), "", RDFFormat.TURTLE);
                    executeRules(writeConnection);
                    writeConnection.commit();
                } catch (Exception e) {
                    if (writeConnection.isActive()) {
                        writeConnection.rollback();
                    }
                    logger.error("Error during data ingestion", e);
                }
            }
        });
    }

    // Blocks until all previously submitted ingestion tasks have committed
    private void awaitPendingTasks() {
        try {
            reasoningExecutor.submit(() -> {}).get();
        } catch (InterruptedException | java.util.concurrent.ExecutionException e) {
            Thread.currentThread().interrupt();
            logger.warn("Interrupted while waiting for pending reasoning tasks", e);
        }
    }

    // Runs all compiled SPARQL CONSTRUCT rules and writes inferences to the
    // inferred graph
    private void executeRules(RepositoryConnection conn) {
        IRI inferredContext = conn.getValueFactory().createIRI("http://example.org/inferred");

        // Clear the inferred context before applying rules to avoid duplicate
        // inferences
        conn.clear(inferredContext);
        int ruleIndex = 0;
        for (GraphQuery ruleQuery : precompiledRules) {
            ruleIndex++;

            // Execute the rule and add inferred statements to the inferred context
            try (GraphQueryResult result = ruleQuery.evaluate()) {
                conn.add((Iterable<Statement>) result, inferredContext);
            } catch (Exception e) {
                logger.error("Failed to execute rule #{}", ruleIndex, e);
            }
        }
        logger.debug("Executed {} rules.", ruleIndex);
    }

    public void clearDatastore() {
        awaitPendingTasks();
        synchronized (writeConnection) {
            try {
                writeConnection.begin();
                writeConnection.clear();
                precompiledRules.clear();
                cachedRuleStrings.clear();
                writeConnection.commit();
                logger.info("Datastore cleared.");
            } catch (Exception e) {
                if (writeConnection.isActive()) writeConnection.rollback();
                logger.error("Error clearing datastore", e);
                throw new RuntimeException("Failed to clear datastore", e);
            }
        }
    }

    public String inferJson(String sparqlQuery) {
        awaitPendingTasks();
        try (RepositoryConnection conn = repository.getConnection()) {
            TupleQuery query = conn.prepareTupleQuery(sparqlQuery);
            try (ByteArrayOutputStream out = new ByteArrayOutputStream()) {
                query.evaluate(new SPARQLResultsJSONWriter(out));
                return out.toString(StandardCharsets.UTF_8);
            }
        } catch (Exception e) {
            logger.error("Error executing SPARQL query (JSON)", e);
            return "{\"head\":{\"vars\":[]},\"results\":{\"bindings\":[]}}";
        }
    }

    public String inferTsv(String sparqlQuery) {
        awaitPendingTasks();
        try (RepositoryConnection conn = repository.getConnection()) {
            TupleQuery query = conn.prepareTupleQuery(sparqlQuery);
            try (ByteArrayOutputStream out = new ByteArrayOutputStream()) {
                query.evaluate(new SPARQLResultsTSVWriter(out));
                return out.toString(StandardCharsets.UTF_8);
            }
        } catch (Exception e) {
            logger.error("Error executing SPARQL query (TSV)", e);
            return "";
        }
    }

    // Sorts rules by sh:order for deterministic execution
    private static class Rule implements Comparable<Rule> {
        final String queryString;
        final double order;

        Rule(String queryString, double order) {
            this.queryString = queryString;
            this.order = order;
        }

        @Override
        public int compareTo(Rule other) {
            return Double.compare(this.order, other.order);
        }
    }

    @PreDestroy
    public void cleanUp() {
        logger.info("Shutting down reasoning executor...");
        reasoningExecutor.shutdown();
        try {
            if (!reasoningExecutor.awaitTermination(5, TimeUnit.SECONDS)) {
                reasoningExecutor.shutdownNow();
            }
        } catch (InterruptedException e) {
            reasoningExecutor.shutdownNow();
        }
        if (writeConnection != null && writeConnection.isOpen()) {
            writeConnection.close();
        }
    }
}
