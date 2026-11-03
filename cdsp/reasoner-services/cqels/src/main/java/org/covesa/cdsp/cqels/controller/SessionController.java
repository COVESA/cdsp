package org.covesa.cdsp.cqels.controller;

import org.covesa.cdsp.cqels.service.SessionRegistry;
import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.DeleteMapping;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RestController;

/** Health check and session (datastore) handling. */
@RestController
public class SessionController {

    private final SessionRegistry sessions;

    /** @param sessions the sessions shared by all endpoints */
    public SessionController(SessionRegistry sessions) {
        this.sessions = sessions;
    }

    /**
     * Health check for Docker Compose.
     *
     * @return 200 with "OK"
     */
    @GetMapping("/health")
    public ResponseEntity<String> health() {
        return ResponseEntity.ok("OK");
    }

    /**
     * Lists the sessions. The Knowledge Layer adapter checks whether its datastore
     * name is in this text.
     *
     * @return the session names, separated by ", " like the RDF4J service
     */
    @GetMapping(value = "/datastores", produces = MediaType.TEXT_PLAIN_VALUE)
    public ResponseEntity<String> list() {
        return ResponseEntity.ok(String.join(", ", sessions.names()));
    }

    /**
     * Creates a session. Calling it for an existing session is not an error.
     *
     * @param name the session (datastore) name
     * @return 200 with "created" or "already exists"
     */
    @PostMapping("/datastores/{name}")
    public ResponseEntity<String> create(@PathVariable String name) {
        boolean created = sessions.create(name);
        return ResponseEntity.ok("Datastore " + name + (created ? " created." : " already exists."));
    }

    /**
     * Deletes a session with its data, streams and queries.
     *
     * @param name the session (datastore) name
     * @return 200 if deleted, 404 if the session does not exist
     */
    @DeleteMapping("/datastores/{name}")
    public ResponseEntity<String> delete(@PathVariable String name) {
        return sessions.delete(name)
                ? ResponseEntity.ok("Datastore " + name + " deleted.")
                : ResponseEntity.notFound().build();
    }
}
