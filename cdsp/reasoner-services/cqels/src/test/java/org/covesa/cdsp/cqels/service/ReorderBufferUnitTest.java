package org.covesa.cdsp.cqels.service;

import static org.junit.jupiter.api.Assertions.assertEquals;

import java.util.List;
import org.junit.jupiter.api.Test;

class ReorderBufferUnitTest {

    private final ReorderBuffer buffer = new ReorderBuffer("Signals", 500);

    /** Adds an element with this event time, arriving at wall-clock time now. */
    private List<Long> add(long eventTime, long now) {
        return times(buffer.add(new ReorderBuffer.Element(List.of(), eventTime), now));
    }

    private List<Long> releaseDue(long now) {
        return times(buffer.releaseDue(now));
    }

    private static List<Long> times(List<ReorderBuffer.Element> elements) {
        return elements.stream().map(ReorderBuffer.Element::eventTime).toList();
    }

    @Test
    void elementsAreReleasedInOrderAndTheNewestAfterWaitingTheTolerance() {
        assertEquals(List.of(), add(2000, 0));
        assertEquals(List.of(), add(1700, 0));
        assertEquals(List.of(1700L, 2000L), add(2600, 0));
        assertEquals(List.of(2600L), add(3100, 0));
        // no newer element comes, so the newest one leaves after the tolerance in real time
        assertEquals(List.of(3100L), releaseDue(500));
    }

    @Test
    void lastElementIsReleasedAfterWaitingTheToleranceInRealTime() {
        add(1000, 0);
        assertEquals(List.of(), releaseDue(499));
        assertEquals(List.of(1000L), releaseDue(500));
        assertEquals(List.of(), releaseDue(1000));
    }

    @Test
    void realTimeReleaseKeepsTheEventTimeOrder() {
        add(2000, 0);
        add(1700, 300);
        // 2000 has waited long enough, so 1700 before it goes as well
        assertEquals(List.of(1700L, 2000L), releaseDue(500));
    }

    @Test
    void elementOlderThanAReleasedOneIsPassedOnAtOnce() {
        add(1000, 0);
        releaseDue(500);
        assertEquals(List.of(800L), add(800, 600));
    }

    @Test
    void zeroToleranceReleasesEveryElementAtOnce() {
        ReorderBuffer direct = new ReorderBuffer("Signals", 0);
        assertEquals(1, direct.add(new ReorderBuffer.Element(List.of(), 1000), 0).size());
    }
}
