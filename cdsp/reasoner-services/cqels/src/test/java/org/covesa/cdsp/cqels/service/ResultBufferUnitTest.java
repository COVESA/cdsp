package org.covesa.cdsp.cqels.service;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertTrue;

import java.util.List;
import java.util.Map;
import org.junit.jupiter.api.Test;

class ResultBufferUnitTest {

    @Test
    void resultsAreReturnedOnlyOnce() {
        ResultBuffer buffer = new ResultBuffer(List.of("x"), 10);
        buffer.add(Map.of("x", 1));
        buffer.add(Map.of("x", 2));

        assertEquals(List.of(Map.of("x", 1), Map.of("x", 2)), buffer.drain());
        assertTrue(buffer.drain().isEmpty());
    }

    @Test
    void fullBufferDropsTheOldestResults() {
        ResultBuffer buffer = new ResultBuffer(List.of("x"), 2);
        buffer.add(Map.of("x", 1));
        buffer.add(Map.of("x", 2));
        buffer.add(Map.of("x", 3));

        assertEquals(List.of(Map.of("x", 2), Map.of("x", 3)), buffer.drain());
    }
}
