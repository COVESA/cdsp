# Example Guidelines
Note: The following text originates from a Christian Muhlbauer proposal for the creation of Knowledge and Information Layer examples. It is reproduced here as the general requirements, concept and approach can be applied to all examples as a general guideline.

## Requirements

1. All adjustable service configurations should be separated from the actual code in the examples folder.

2. It should be easy to run a specific use-case and validate its results.

3. A default configuration should be used, if all services are run without a specific use-cases (from the docker folder).

## Concept
```
root
├── cdsp
│   ├── information-layer
│   │   └── Dockerfile
│   └── knowledge-layer
│       └── Dockerfile
├── docker
│   └── docker-compose-cdsp.yml
└── examples
    ├── aggressive-driving
    │   ├── docker-compose-aggressive-driving.yml
    │   ├── KL-config
    │   │   ├── model_config.json
    │   │   ├── driving_style_inference_rules.dlog
    │   │   ├── observation_shacl.ttl
    │   │   ├── vehicle_data_input.txt
    │   │   ├── ontology.ttl
    │   │   └── data_property.rq
    │   ├── IL-config
    │   │   └── vss_data_points_new.yaml
    │   └── results
    └── other-example
        └── ....
```

### CDSP Core Services
The cdsp/ directory contains services with its own Docker file for containerized deployment:
+ **Information Layer (IL)**: Validates, stores and distributes VSS data points
+ **Knowledge Layer (KL)**: Consumes relevant VSS data points from **IL**, applies reasoning on the data and writes results into files and back to the **IL**

### Docker Folder
The docker/ directory contains docker-compose configurations for running all services together. The file docker-compose-cdsp.yml defines how these services interact and enables seamless orchestration.
Running docker compose up from there, should be enough to start everything with default configuration.

### Example Use Cases
The examples/ directory contains specific real-world test scenarios, each demonstrating the CDSP system in action, e.g.:

+ **aggressive-driving/**: A scenario focused on detecting and analyzing aggressive driving behavior.

+ **parking-behavior-detection/**: Detecting repeating parking behavior and taking over repetitive tasks.

+ **other-example/**: Additional test cases that showcase different functionalities of the system.

Each example directory contains:

+ **docker-compose.yml**: Extends the main docker/docker-compose-cdsp.yml file without duplicating default configurations to keep things modular and maintainable. Naming of the service can reflect the use-case and configuration. ex. *Information Layer Server IotDB* vs. *Information Layer Server RealmDB*

+ **KL-config/**: Folder with scenario specific rules, validations, queries, etc. and a JSON config file for finding them and configuration. Will be mounted into the Knowledge-Layer on service startup.

+ **IL-config/**: Folder with scenario specific config (ex. VSS data points) to replace/extend the default one in case new data points are needed. Will be mounted into the Information-Layer on service startup.

+ **result/**: Folder for storing information about detected behavior. Will be mounted to the KL output folder.

### How It Works
1. Use **Docker Compose** from one example folder to start all required services for the use-case.

2. Each example provides **KL** and/or **IL** configuration defining the overall systems behavior.

3. The **results** folder in each example is mounted to the **knowledge layer**, allowing persistent storage and easy access to generated outputs.

4. After execution, the results can be reviewed directly within the respective **results** folder.