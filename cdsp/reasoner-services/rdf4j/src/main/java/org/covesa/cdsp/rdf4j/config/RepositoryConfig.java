package org.covesa.cdsp.rdf4j.config;

import org.eclipse.rdf4j.repository.Repository;
import org.eclipse.rdf4j.repository.sail.SailRepository;
import org.eclipse.rdf4j.sail.memory.MemoryStore;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;

@Configuration
public class RepositoryConfig {

    private static final Logger logger = LoggerFactory.getLogger(RepositoryConfig.class);

    @Bean
    public Repository repository() {
        logger.info("Initializing RDF4J MemoryStore...");
        Repository repo = new SailRepository(new MemoryStore());
        repo.init();
        logger.info("RDF4J repository initialized.");
        return repo;
    }
}
