Here is a sample `README.md` that explains the structure and usage of the `model_config.json` configuration file.

---

# Model Configuration for Reasoner Application

This `model_config.json` file provides the configuration necessary to run the reasoner application. The file defines the required inputs, ontologies, output settings, queries, rules, SHACL validation, and reasoning engine options. This document explains the purpose of each section in the configuration.

## Configuration Overview

The `model_config.json` is structured to provide:
- Input data points
- Ontology files for the domain schema
- SHACL shapes for data validation
- SPARQL queries for data manipulation
- Reasoning rules
- Reasoner settings
- Output configurations

### Configuration Sections

#### Inputs
```json
"inputs": {
  "vss_data": "inputs/vss_data_required.txt"
}
```
- **vss_data**: The field must be called `<tree_type>_data`, in the example we use the VSS tree type (`vss_data`). This field specifies the path to the input data file that contains vehicle signals or data points. This file lists the signals that the reasoner application will work with. It supports a TXT format for defining the data points.

#### Ontologies
```json
"ontologies": ["ontologies/example_ontology.ttl"]
```
- **ontologies**: A list of ontology files in Turtle format (.ttl). These define the concepts, relationships, and structure of the data in a semantic format that the reasoner will use to understand and process the input data.

#### Output
```json
"output": "output/generated_triples.ttl"
```
- **output**: Defines the path to the file where the reasoner will store the results of the inference process. The output will be generated in the format defined in the [reasoner settings](#reasoner-settings) (in this case a turtle `.ttl`).
  > [!IMPORTANT] Do not add the generated files to the repository
  > - The generated files are into `.gitignore` file, they must be use locally only.

#### Queries
```json
"queries": {
  "triple_assembler_helper": {
    "vss": [
      "queries/triple_assembler_helper/vss/data_property.rq",
      "queries/triple_assembler_helper/vss/object_property.rq"
    ],
    "default": [
        "queries/triple_assembler_helper/default/data_property.rq",
        "queries/triple_assembler_helper/default/object_property.rq"
    ]
  },
  "output": ["queries/output/"]
}
```
- **queries**: This section includes SPARQL queries that will be used during the reasoning process to assemble triples or extract results from the reasoned data.
  - **triple_assembler_helper**: 
    Queries specifically designed to assemble data points related to an specific data tree (in this case Vehicle Signal Specification (VSS)) or other specifications used by default, if the tree is not defined, including queries for data and object properties.
  - **output**: Queries to retrieve the final inference results after the reasoning process. The queries in this section will typically extract insights from the generated triples.

#### Rules
```json
"rules": ["rules/insight_rules.ttl"]
```
- **rules**: A list of rule files in Turtle format. These rules define additional logic for the reasoner to apply during inference. The rules may include custom inferences or insights that the reasoner should derive based on the input data.

#### SHACL Validation
```json
"shacl": [
  "shacl/vss_shacl.ttl",
  "shacl/observation_shacl.ttl"
]
```
- **shacl**: A list of SHACL (Shapes Constraint Language) files that define the constraints and validation rules for the data. SHACL shapes ensure that the input data conforms to specific structural and semantic rules before it is processed by the reasoner.

#### Reasoner Settings
```json
"reasoner_settings": {
  "inference_engine": "RDFox",
  "output_format": "turtle",
  "supported_tree_types": ["VSS"]
}
```
- **reasoner_settings**: Configuration options for the reasoning engine.
  - **inference_engine**: Specifies the reasoner to use (in this case, RDFox).
    > [!NOTE] Supported engines in this repository
    > - `RDFox`

  - **output_format**: Defines the format in which the output will be serialized. The current setting is `turtle` for Turtle format.
    > [!NOTE] Supported formats in this repository
    > - `turtle` for .ttl files

  - **supported_tree_types**: This field defines the types of data trees that the reasoner application can handle. For the communication with the WebSocket server and definition of the input data, it is required to specify how the tree type will be represented. This tree type will be sent as part of the message header during data transfer and is crucial for reading, interpreting the [input data](#inputs) for the reasoner model, and if exist, specific [triple assembler helpers](#queries). With this tree type definition, the reasoner can ensure compatibility with the input data and properly interpret the data points being processed.
    > [!NOTE] Supported tree types this repository
    > - `VSS` (Vehicle Signal Specification)

### Example Usage

Once the `model_config.json` is configured, the reasoner application will:
1. Read the **input data** from the specified file.
2. Validate the data using the **ontologies** and **SHACL shapes**.
3. Apply the **rules** during inference.
4. Use the **queries** to assemble triples and extract results.
5. Generate an **output** file containing the reasoned data in Turtle format.