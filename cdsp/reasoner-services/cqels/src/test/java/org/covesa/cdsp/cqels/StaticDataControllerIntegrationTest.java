package org.covesa.cdsp.cqels;

import static org.hamcrest.Matchers.containsString;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.delete;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.post;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.content;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.header;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.jsonPath;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.status;

import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureMockMvc;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.test.web.servlet.MockMvc;

@SpringBootTest
@AutoConfigureMockMvc
class StaticDataControllerIntegrationTest {

    private static final String TURTLE = "<http://ex.org/car> a <http://ex.org/Vehicle> .";
    private static final String QUERY = "SELECT ?s WHERE { ?s a <http://ex.org/Vehicle> }";

    @Autowired
    private MockMvc mvc;

    @BeforeEach
    void createSession() throws Exception {
        mvc.perform(post("/datastores/ds-static"));
    }

    @AfterEach
    void deleteSession() throws Exception {
        mvc.perform(delete("/datastores/ds-static"));
    }

    @Test
    void loadedDataIsAnsweredAsTsv() throws Exception {
        mvc.perform(post("/datastores/ds-static/content").contentType("text/turtle").content(TURTLE))
                .andExpect(status().isOk());
        mvc.perform(post("/datastores/ds-static/sparql").contentType("application/sparql-query")
                        .accept("text/tab-separated-values").content(QUERY))
                .andExpect(status().isOk())
                .andExpect(header().string("Content-Type", containsString("text/tab-separated-values")))
                .andExpect(content().string("?s\n<http://ex.org/car>\n"));
    }

    @Test
    void loadedDataIsAnsweredAsSparqlJson() throws Exception {
        mvc.perform(post("/datastores/ds-static/content").contentType("text/turtle").content(TURTLE));
        mvc.perform(post("/datastores/ds-static/sparql").contentType("application/sparql-query")
                        .accept("application/sparql-results+json").content(QUERY))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.results.bindings[0].s.value").value("http://ex.org/car"));
    }

    @Test
    void invalidInputIsRejected() throws Exception {
        mvc.perform(post("/datastores/ds-static/content").contentType("text/turtle").content("not turtle"))
                .andExpect(status().isBadRequest());
        mvc.perform(post("/datastores/ds-static/sparql").contentType("application/sparql-query").content("SELECT"))
                .andExpect(status().isBadRequest());
    }

    @Test
    void unknownSessionIsNotFound() throws Exception {
        mvc.perform(post("/datastores/unknown/content").contentType("text/turtle").content(TURTLE))
                .andExpect(status().isNotFound());
        mvc.perform(post("/datastores/unknown/sparql").contentType("application/sparql-query").content(QUERY))
                .andExpect(status().isNotFound());
    }
}
