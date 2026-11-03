package org.covesa.cdsp.cqels.service;

import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.Deque;
import java.util.List;
import java.util.Map;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * Results of one continuous query that the Knowledge Layer has not fetched yet.
 * When the buffer is full, the oldest result is dropped.
 */
class ResultBuffer {

    private static final Logger logger = LoggerFactory.getLogger(ResultBuffer.class);
    private final int maxSize;
    private final Deque<Map<String, Object>> results = new ArrayDeque<>();
    private boolean dropping = false;

    /**
     * The query the results belong to, for the log; set once the engine returns the
     * id.
     */
    volatile String queryId;

    /** The query's SELECT variables, for the header of the results. */
    final List<String> variables;

    /**
     * @param variables the query's SELECT variables
     * @param maxSize   the maximum number of results kept
     */
    ResultBuffer(List<String> variables, int maxSize) {
        this.variables = variables;
        this.maxSize = maxSize;
    }

    /**
     * Adds a result; drops the oldest one if the buffer is full.
     *
     * @param result one result row, variable name to value
     */
    synchronized void add(Map<String, Object> result) {
        if (results.size() >= maxSize) {
            results.removeFirst();
            if (!dropping) {
                logger.warn("Results buffer of query {} is full ({}), dropping the oldest results", queryId, maxSize);
                dropping = true;
            }
        }
        results.addLast(result);
    }

    /**
     * Takes all buffered results, so each result is returned only once.
     *
     * @return the results since the last call, oldest first
     */
    synchronized List<Map<String, Object>> drain() {
        List<Map<String, Object>> drained = new ArrayList<>(results);
        results.clear();
        dropping = false;
        return drained;
    }
}
