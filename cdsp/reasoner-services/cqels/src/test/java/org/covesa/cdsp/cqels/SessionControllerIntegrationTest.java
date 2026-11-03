package org.covesa.cdsp.cqels;

import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.delete;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.get;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.post;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.content;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.status;

import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.webmvc.test.autoconfigure.AutoConfigureMockMvc;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.test.web.servlet.MockMvc;

@SpringBootTest
@AutoConfigureMockMvc
class SessionControllerIntegrationTest {

    @Autowired
    private MockMvc mvc;

    @Test
    void healthIsOk() throws Exception {
        mvc.perform(get("/health")).andExpect(status().isOk());
    }

    @Test
    void createListAndDeleteSession() throws Exception {
        mvc.perform(post("/datastores/ds-a")).andExpect(status().isOk());
        mvc.perform(post("/datastores/ds-a"))
                .andExpect(status().isOk())
                .andExpect(content().string("Datastore ds-a already exists."));
        mvc.perform(get("/datastores")).andExpect(content().string(org.hamcrest.Matchers.containsString("ds-a")));

        mvc.perform(delete("/datastores/ds-a")).andExpect(status().isOk());
        mvc.perform(get("/datastores"))
                .andExpect(content().string(org.hamcrest.Matchers.not(org.hamcrest.Matchers.containsString("ds-a"))));
    }

    @Test
    void deletingAnUnknownSessionIsNotFound() throws Exception {
        mvc.perform(delete("/datastores/unknown")).andExpect(status().isNotFound());
    }
}
