package org.covesa.cdsp.cqels;

import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.scheduling.annotation.EnableScheduling;

/**
 * REST service that runs the CQELS stream reasoner for the CDSP Knowledge
 * Layer.
 */
@SpringBootApplication
@EnableScheduling
public class CqelsReasonerApplication {

    /**
     * Starts the service on the port in application.properties (SERVER_PORT
     * overrides it).
     *
     * @param args command-line arguments, passed on to Spring Boot
     */
    public static void main(String[] args) {
        SpringApplication.run(CqelsReasonerApplication.class, args);
    }
}
