package org.covesa.cdsp.cqels.service;

import java.time.Instant;
import java.util.Collection;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.concurrent.ConcurrentHashMap;
import org.cqels.engine.CQELSEngine;
import org.cqels.lang.CqelsQueryDefinition;
import org.cqels.lang.CqelsQueryParser;
import org.eclipse.rdf4j.model.Statement;

/**
 * One session: a CQELS engine and the result buffers of its continuous queries.
 */
public class Session {

    private final CQELSEngine engine;
    private final int resultsBufferSize;
    private final long reorderToleranceMillis;
    private final Map<String, ResultBuffer> buffers = new ConcurrentHashMap<>();
    private final Map<String, ReorderBuffer> reorderBuffers = new ConcurrentHashMap<>();
    private boolean closed = false;

    /**
     * @param engine                 the session's own CQELS engine
     * @param resultsBufferSize      the maximum number of unfetched results kept
     *                               per query
     * @param reorderToleranceMillis how much later than a newer element an element
     *                               may arrive and still be used
     */
    Session(CQELSEngine engine, int resultsBufferSize, long reorderToleranceMillis) {
        this.engine = engine;
        this.resultsBufferSize = resultsBufferSize;
        this.reorderToleranceMillis = reorderToleranceMillis;
    }

    /**
     * Gives access to the engine, for example to its store of static data.
     *
     * @return the session's CQELS engine
     */
    public CQELSEngine engine() {
        return engine;
    }

    /**
     * Registers a continuous query; its results are buffered until they are
     * fetched. The streams the query reads are created first, because the engine
     * only accepts queries on existing streams.
     *
     * @param cqelsQl the query in CQELS-QL
     * @return the query id
     * @throws RuntimeException if the engine rejects the query
     */
    public String registerQuery(String cqelsQl) {
        CqelsQueryDefinition definition = CqelsQueryParser.parseCqelsQL(cqelsQl);
        definition.getStreams().keySet().forEach(engine::getOrCreateStream);
        ResultBuffer buffer = new ResultBuffer(definition.getSelectVariables(), resultsBufferSize);
        String id = engine.registerCqelsQuery(cqelsQl, buffer::add);
        buffer.queryId = id;
        buffers.put(id, buffer);
        return id;
    }

    /**
     * Pushes one stream element. It reaches the engine in event-time order, once
     * the stream is the reorder tolerance ahead of it or it has waited the
     * tolerance in real time. The engine starts with the first element, after the
     * queries are registered.
     *
     * @param stream     the stream name used in the queries
     * @param statements the triples of the element
     * @param eventTime  the time the element was observed
     */
    public synchronized void push(String stream, Collection<Statement> statements, Instant eventTime) {
        if (!engine.isRunning()) {
            engine.start();
        }
        ReorderBuffer reorder = reorderBuffers.computeIfAbsent(stream,
                s -> new ReorderBuffer(s, reorderToleranceMillis));
        toEngine(stream, reorder.add(new ReorderBuffer.Element(statements, eventTime.toEpochMilli()),
                System.currentTimeMillis()));
    }

    /**
     * Pushes the held elements that have waited the reorder tolerance in real
     * time. Called regularly, so the last element of a stream is not held until a
     * newer one arrives.
     *
     * @param now the current wall-clock time in epoch milliseconds
     */
    synchronized void releaseDue(long now) {
        if (closed) {
            return;
        }
        reorderBuffers.forEach((stream, reorder) -> toEngine(stream, reorder.releaseDue(now)));
    }

    /**
     * Pushes released elements to the engine.
     *
     * @param stream   the stream name
     * @param elements the elements, oldest first
     */
    private void toEngine(String stream, List<ReorderBuffer.Element> elements) {
        for (ReorderBuffer.Element element : elements) {
            engine.getOrCreateStream(stream).push(element.statements(), element.eventTime());
        }
    }

    /**
     * Takes the results of a query since the last call.
     *
     * @param queryId the id returned at registration
     * @return the query's variables and results, or empty if the query is unknown
     */
    public Optional<Results> drainResults(String queryId) {
        return Optional.ofNullable(buffers.get(queryId)).map(b -> new Results(b.variables, b.drain()));
    }

    /**
     * Results of a query since the last call.
     *
     * @param variables the query's SELECT variables
     * @param rows      the result rows, oldest first, variable name to value
     */
    public record Results(List<String> variables, List<Map<String, Object>> rows) {
    }

    /**
     * Closes the engine with its streams and queries. Held stream elements are
     * dropped, because nobody can fetch their results any more.
     */
    synchronized void close() {
        closed = true;
        engine.close();
    }
}
