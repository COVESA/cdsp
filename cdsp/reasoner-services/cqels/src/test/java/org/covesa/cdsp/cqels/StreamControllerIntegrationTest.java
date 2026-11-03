package org.covesa.cdsp.cqels;

import static org.junit.jupiter.api.Assertions.assertTrue;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.delete;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.get;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.post;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.status;

import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.webmvc.test.autoconfigure.AutoConfigureMockMvc;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.test.web.servlet.MockMvc;

@SpringBootTest
@AutoConfigureMockMvc
class StreamControllerIntegrationTest {

    private static final String QUERY = """
            PREFIX ex: <http://ex.org/>
            REGISTER QUERY Speed AS
            SELECT ?car ?speed
            FROM STREAM Signals [RANGE 5s]
            WHERE { STREAM Signals { ?car ex:speed ?speed } }
            """;

    @Autowired
    private MockMvc mvc;

    @BeforeEach
    void createSession() throws Exception {
        mvc.perform(post("/datastores/ds-stream"));
    }

    @AfterEach
    void deleteSession() throws Exception {
        mvc.perform(delete("/datastores/ds-stream"));
    }

    private String register(String query) throws Exception {
        return mvc.perform(post("/datastores/ds-stream/queries").contentType("text/plain").content(query))
                .andExpect(status().isOk())
                .andReturn().getResponse().getContentAsString();
    }

    private void push(String eventTime, String turtle) throws Exception {
        mvc.perform(post("/datastores/ds-stream/streams/Signals").param("eventTime", eventTime)
                .contentType("text/turtle").content(turtle))
                .andExpect(status().isOk());
    }

    private String results(String id) throws Exception {
        return mvc.perform(get("/datastores/ds-stream/queries/" + id + "/results"))
                .andExpect(status().isOk())
                .andReturn().getResponse().getContentAsString();
    }

    /** Collects the results of a query until they contain the value, for at most 5 s. */
    private String awaitValue(String id, String value) throws Exception {
        StringBuilder all = new StringBuilder();
        for (int i = 0; i < 50 && !all.toString().contains("\"" + value + "\""); i++) {
            all.append(results(id));
            Thread.sleep(100);
        }
        return all.toString();
    }

    /**
     * The last element of a stream gives its result without a newer element: the
     * reorder buffer releases it after the tolerance in real time, together with a
     * late element it still holds.
     */
    @Test
    void lastPushedElementGivesResultsWithoutANewerOne() throws Exception {
        String id = register(QUERY);
        push("2026-10-30T10:00:00.000Z", "<http://ex.org/car1> <http://ex.org/speed> 100 .");
        push("2026-10-30T10:00:01.000Z", "<http://ex.org/car1> <http://ex.org/speed> 120 .");
        push("2026-10-30T10:00:00.700Z", "<http://ex.org/car1> <http://ex.org/speed> 110 .");
        push("2026-10-30T10:00:01.300Z", "<http://ex.org/car1> <http://ex.org/speed> 130 .");

        String all = awaitValue(id, "130");
        assertTrue(all.contains("\"110\"") && all.contains("\"130\""), all);
    }

    /**
     * Signals reach the server one by one, so an element can arrive after a newer one. Once the
     * engine has processed the newer one, it drops the late element; the reorder buffer (500 ms by
     * default) pushes the elements in order, so the late one still counts. Fails with a tolerance
     * of 0 ms.
     */
    @Test
    void lateElementWithinTheToleranceIsStillUsed() throws Exception {
        String id = register("""
                PREFIX ex: <http://ex.org/>
                REGISTER QUERY SpeedSum AS
                SELECT ?car (SUM(?speed) AS ?sum)
                FROM STREAM Signals [RANGE 2s]
                WHERE { STREAM Signals { ?car ex:speed ?speed } }
                GROUP BY ?car
                """);
        push("2026-10-30T10:00:00.000Z", "<http://ex.org/car1> <http://ex.org/speed> 1 .");
        push("2026-10-30T10:00:01.000Z", "<http://ex.org/car1> <http://ex.org/speed> 10 .");
        Thread.sleep(300); // let the engine process the newer element first
        push("2026-10-30T10:00:00.700Z", "<http://ex.org/car1> <http://ex.org/speed> 100 .");
        push("2026-10-30T10:00:02.000Z", "<http://ex.org/car1> <http://ex.org/speed> 1000 .");
        push("2026-10-30T10:00:03.000Z", "<http://ex.org/car1> <http://ex.org/speed> 10000 .");

        // 1 + 100 + 10: the window up to 10:00:02 with the late element
        String all = awaitValue(id, "111");
        assertTrue(all.contains("\"111\""), all);
    }

    @Test
    void invalidInputIsRejected() throws Exception {
        mvc.perform(post("/datastores/ds-stream/queries").contentType("text/plain").content("REGISTER nonsense"))
                .andExpect(status().isBadRequest());
        mvc.perform(post("/datastores/ds-stream/streams/Signals").param("eventTime", "yesterday")
                .contentType("text/turtle").content("<http://ex.org/a> <http://ex.org/b> 1 ."))
                .andExpect(status().isBadRequest());
        mvc.perform(post("/datastores/ds-stream/streams/Signals")
                .contentType("text/turtle").content("<http://ex.org/a> <http://ex.org/b> 1 ."))
                .andExpect(status().isBadRequest());
        mvc.perform(post("/datastores/ds-stream/streams/Signals").param("eventTime", "2026-10-30T10:00:00Z")
                .contentType("text/turtle").content("not turtle"))
                .andExpect(status().isBadRequest());
    }

    @Test
    void unknownSessionOrQueryIsNotFound() throws Exception {
        mvc.perform(get("/datastores/ds-stream/queries/unknown/results")).andExpect(status().isNotFound());
        mvc.perform(post("/datastores/unknown/queries").contentType("text/plain").content(QUERY))
                .andExpect(status().isNotFound());
        mvc.perform(post("/datastores/unknown/streams/Signals").param("eventTime", "2026-10-30T10:00:00Z")
                .contentType("text/turtle").content("<http://ex.org/a> <http://ex.org/b> 1 ."))
                .andExpect(status().isNotFound());
    }
}
