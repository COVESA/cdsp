package org.covesa.cdsp.cqels.service;

import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Deque;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;
import org.eclipse.rdf4j.model.Statement;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * Puts the elements of one stream back in event-time order before they reach
 * the engine, which drops late elements silently. An element is held until the
 * newest event time seen is at least the tolerance ahead of it, or until it has
 * waited the tolerance in real time, so the last element of a stream is not
 * held forever.
 */
class ReorderBuffer {

    private static final Logger logger = LoggerFactory.getLogger(ReorderBuffer.class);
    private final String stream;
    private final long toleranceMillis;
    private final TreeMap<Long, Deque<Held>> held = new TreeMap<>();
    private long newest = Long.MIN_VALUE;
    private long lastReleased = Long.MIN_VALUE;

    /**
     * One stream element.
     *
     * @param statements the triples of the element
     * @param eventTime  the observation time in epoch milliseconds
     */
    record Element(Collection<Statement> statements, long eventTime) {
    }

    /**
     * An element waiting in the buffer.
     *
     * @param element   the element
     * @param arrivedAt the wall-clock time it arrived, in epoch milliseconds
     */
    private record Held(Element element, long arrivedAt) {
    }

    /**
     * @param stream          the stream name, for the log
     * @param toleranceMillis how much later than a newer element an element may
     *                        arrive
     */
    ReorderBuffer(String stream, long toleranceMillis) {
        this.stream = stream;
        this.toleranceMillis = toleranceMillis;
    }

    /**
     * Adds an element and takes the elements that are ready for the engine.
     *
     * @param element the new element
     * @param now     the current wall-clock time in epoch milliseconds
     * @return the elements to push now, oldest first; an element older than one
     *         already released is returned at once, and the engine may drop it
     */
    List<Element> add(Element element, long now) {
        if (element.eventTime() < lastReleased) {
            logger.warn("Element of stream {} at {} arrived later than the tolerance of {} ms; the engine may drop it",
                    stream, element.eventTime(), toleranceMillis);
            return List.of(element);
        }
        held.computeIfAbsent(element.eventTime(), t -> new ArrayDeque<>()).addLast(new Held(element, now));
        newest = Math.max(newest, element.eventTime());
        return releaseUpTo(newest - toleranceMillis);
    }

    /**
     * Takes the elements that have waited the tolerance in real time, together
     * with all held elements before them in event time, so the order is kept.
     *
     * @param now the current wall-clock time in epoch milliseconds
     * @return the elements to push now, oldest first
     */
    List<Element> releaseDue(long now) {
        long upTo = Long.MIN_VALUE;
        for (Map.Entry<Long, Deque<Held>> entry : held.entrySet()) {
            for (Held h : entry.getValue()) {
                if (now - h.arrivedAt() >= toleranceMillis) {
                    upTo = entry.getKey();
                }
            }
        }
        return releaseUpTo(upTo);
    }

    /**
     * Takes all held elements up to an event time.
     *
     * @param eventTime the newest event time to release
     * @return the released elements, oldest first
     */
    private List<Element> releaseUpTo(long eventTime) {
        List<Element> ready = new ArrayList<>();
        while (!held.isEmpty() && held.firstKey() <= eventTime) {
            lastReleased = held.firstKey();
            held.pollFirstEntry().getValue().forEach(h -> ready.add(h.element()));
        }
        return ready;
    }
}
