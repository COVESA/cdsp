package org.covesa.cdsp.cqels.service;

import jakarta.annotation.PreDestroy;
import java.time.Duration;
import java.util.Map;
import java.util.Optional;
import java.util.Set;
import java.util.TreeSet;
import java.util.concurrent.ConcurrentHashMap;
import org.cqels.engine.CQELSEngine;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.scheduling.annotation.Scheduled;
import org.springframework.stereotype.Service;

/**
 * Keeps one session with its own CQELS engine per datastore name. Deleting a
 * session closes its engine, which also removes its streams and continuous
 * queries.
 */
@Service
public class SessionRegistry {

    private static final Logger logger = LoggerFactory.getLogger(SessionRegistry.class);
    private final Map<String, Session> sessions = new ConcurrentHashMap<>();
    private final int resultsBufferSize;
    private final Duration reorderTolerance;

    /**
     * @param resultsBufferSize the maximum number of unfetched results kept per
     *                          query
     * @param reorderTolerance  how much later than a newer element an element may
     *                          arrive and still be used
     */
    public SessionRegistry(@Value("${cqels.results-buffer-size}") int resultsBufferSize,
            @Value("${cqels.reorder-tolerance}") Duration reorderTolerance) {
        this.resultsBufferSize = resultsBufferSize;
        this.reorderTolerance = reorderTolerance;
    }

    /**
     * Creates the session with its own CQELS engine, if it does not exist yet.
     *
     * @param name the session (datastore) name
     * @return true if the session was created, false if it already existed
     */
    public boolean create(String name) {
        boolean[] created = { false };
        sessions.computeIfAbsent(name, n -> {
            created[0] = true;
            logger.info("Creating session {}", n);
            return new Session(CQELSEngine.builder().id(n).withMemoryStore().build(), resultsBufferSize,
                    reorderTolerance.toMillis());
        });
        return created[0];
    }

    /**
     * Closes the session's engine and removes the session.
     *
     * @param name the session (datastore) name
     * @return true if the session was deleted, false if it did not exist
     */
    public boolean delete(String name) {
        Session session = sessions.remove(name);
        if (session == null) {
            return false;
        }
        logger.info("Deleting session {}", name);
        session.close();
        return true;
    }

    /**
     * Looks up a session.
     *
     * @param name the session (datastore) name
     * @return the session, or empty if it does not exist
     */
    public Optional<Session> get(String name) {
        return Optional.ofNullable(sessions.get(name));
    }

    /**
     * Lists the existing sessions.
     *
     * @return the session names, sorted
     */
    public Set<String> names() {
        return new TreeSet<>(sessions.keySet());
    }

    /**
     * Pushes held stream elements that have waited the reorder tolerance in real
     * time, in every session.
     */
    // ponytail: fixed 100 ms tick, so an element waits at most the tolerance plus 100 ms
    @Scheduled(fixedDelay = 100)
    void releaseDueElements() {
        long now = System.currentTimeMillis();
        sessions.values().forEach(session -> session.releaseDue(now));
    }

    /** Closes all engines when the service shuts down. */
    @PreDestroy
    void closeAll() {
        sessions.keySet().forEach(this::delete);
    }
}
