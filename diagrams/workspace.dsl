workspace "COVESA Central Data Service Playground (CDSP)" "C4 model diagrams for the COVESA Central Data Service Playground (CDSP) architecture" {

    !identifiers hierarchical

    model {
        knowledgeEngineer = person "Knowledge Engineer & Domain expert" "Provider of domain expertise"

        onboardOffboardApplications = softwareSystem "Onboard and Offboard applications" "Contributing and consuming information and knowledge" {
            tags "External Software System"
        }

        remotiveLabsPlatform = softwareSystem "RemotiveLabs Virtual Signal Platform" "Virtual signal platform in the cloud" {
            tags "External Software System"
        }

        aiAgents = softwareSystem "AI Agents" "E.g. LLM based agent, graph retrieval via MCP" {
            tags "External Software System"
        }

        cdsp = softwareSystem "Central Data Service Playground (CDSP)" "Central data service playground providing information, knowledge, graph retrieval and supporting data services." {

            informationLayerServer = container "CDSP Information Layer Server" "Provides access to information-layer data and abstracts the underlying time-series data store." "Node.js" {
                dbRouter = component "DB Router" "Provides information layer data in tree-like format. Abstracts data store details" "Node.JS Websocket Server"
                iotdbHandler = component "IotDB Handler" "Provides information layer data in tree-like format. Abstracts data store details" "Node.JS Websocket Server"

                dbRouter -> iotdbHandler "(De-)Auth / Read / Write / Subscribe" "Service Call"
            }

            timeseriesDataServer = container "Timeseries Data Server" "Serves timeseries data in VSS and other data-models" "Apache IoTDB" {
                tags "Database"
            }

            knowledgeLayerServer = container "CDSP Knowledge Layer Server" "Converts information-layer data to graph data and provides knowledge-graph reasoning and query capabilities." "C++" {
                convertor = component "Convertor" "Converts tree-like data (json) into graph data (RDF) and vice versa" "C++"
                connector = component "Connector" "Connects Knowledge Layer to Information Layer and manage data tasks within KL" "Websocket Client"
                reasonerAdaptor = component "Reasoner-Adaptor" "Interacts with Graph database and reasoner by API (e.g. allows import of RDF data and rules and execute queries)" "C++"
                rdfReasoner = component "RDF Reasoner Service and Knowledge Graph" "Stores RDF data, executes rules, and evaluates queries." "RDFox or RDF4J"

                reasonerAdaptor -> rdfReasoner "Send triples, rules, queries" "HTTP"
                reasonerAdaptor -> rdfReasoner "Subscribes to new inferred triples" "HTTP"
                reasonerAdaptor -> connector "Inference result (rules)" "HTTP"
                reasonerAdaptor -> connector "Inference result (query)" "HTTP"
                connector -> reasonerAdaptor "Trigger Graph database interactions (ontologies, triples, rules)" "HTTP"
                connector -> convertor "Sends tree-like data (json)" "HTTP"
                convertor -> connector "Sends Graph data (RDF)" "HTTP"
                convertor -> connector "Sends tree-like data (json)" "HTTP"
                connector -> convertor "Sends Graph data (RDF)" "HTTP"
            }

            vectorStorageServer = container "Vector Storage Server" "Ontology Node Embeddings" "e.g. Chroma" {
                tags "Database"
            }

            graphRetrievalMcpServer = container "(WIP) CDSP Graph Retrieval MCP Server" "Provides graph retrieval strategies to AI agents through MCP." "TypeScript" {
                mcpTransport = component "MCP Transport" "Exposes all strategies as MCP tools. Handles Session and Auth" "Streamable HTTP"
                strategyLayer = component "Strategy Layer" "seed_expand, local_fanout, path_find, timeseries_aware + extensible" "TypeScript"
                sharedServices = component "Shared Services" "Semantic Search, Graph Traversal, Live Data, Temporal Analysis" "TypeScript"
                knowledgeGraphAdapter = component "Knowledge Graph Adapter" "query (sparql), traverse (seeds), pathFind(from, to), describe(iri)" "Interface"
                vectorStoreAdapter = component "Vector Store Adapter" "search(query, n), filter(query, metadata)" "Interface"
                informationLayerAdapter = component "Information Layer Adapter" "get(path, instance), subscribe(path), timeseries(path, range)" "Interface"

                mcpTransport -> strategyLayer "Dispatches to selected strategy" "in-process"
                strategyLayer -> sharedServices "Delegates Retrieval Operations" "in-process"
                sharedServices -> vectorStoreAdapter "Semantic Node Search" "in-process"
                sharedServices -> knowledgeGraphAdapter "Graph Queries and Traversal" "in-process"
                sharedServices -> informationLayerAdapter "Live Data and Timeseries" "in-process"
            }

            remotiveLabsBridge = container "RemotiveLabs Bridge" "Streams subscribed timeseries data from RemotiveLabs virtual signal platform to Playground data store" "Python" {
                -> remotiveLabsPlatform "Subscribes to signals" "Remotive API"
                remotiveLabsPlatform -> cdsp.remotiveLabsBridge "Sends signal data timeseries" "Remotive API"
                -> cdsp.timeseriesDataServer "Writes timeseries data stream" "IoTDB API/SQL"                
            }

            vissServer = container "VISS Server" "Serves VSS data via the VISS AP" "VISSR" {
                -> cdsp.timeseriesDataServer "Sends VSS get/set requests to" "IoTDB API/SQL"
            }

            // Information Layer external relationships
            informationLayerServer.dbRouter -> onboardOffboardApplications "Consuming Information" "Websocket API"
            onboardOffboardApplications -> cdsp.informationLayerServer.dbRouter "Contributing Information" "Websocket API"
            informationLayerServer.iotdbHandler -> timeseriesDataServer "Read / Write / (Subscribe)" "Node.js client"
            informationLayerServer.dbRouter -> knowledgeLayerServer.connector "Provides Information" "Websocket API"
            knowledgeEngineer -> cdsp.informationLayerServer.dbRouter "Provides semantic context for later conversion" "e.g. SHACL"
            cdsp.remotiveLabsBridge -> cdsp.informationLayerServer.dbRouter "Writes current value of data stream" "Websocket API"

            // Knowledge Layer external relationships
            knowledgeLayerServer.connector -> onboardOffboardApplications "Consuming Knowledge" "HTTP SPARQL"
            knowledgeLayerServer.connector -> cdsp.informationLayerServer.dbRouter "Returns new Information" "Websocket API"
            knowledgeEngineer -> cdsp.knowledgeLayerServer.reasonerAdaptor "Initialize with ontologies and rules (representing business logic) and update" "HTTP"

            // Graph RAG Server external relationships
            aiAgents -> graphRetrievalMcpServer.mcpTransport "Invokes Graph Retrieval Tool" "MCP Streamable HTTP"
            graphRetrievalMcpServer.informationLayerAdapter -> cdsp.informationLayerServer.dbRouter "Live and historical data" "Websocket API"
            graphRetrievalMcpServer.knowledgeGraphAdapter -> cdsp.knowledgeLayerServer.connector "SPARQL QUERIES" "HTTP SPARQL"
            vectorStorageServer -> graphRetrievalMcpServer.vectorStoreAdapter "Embedding Search" "HTTP REST"
        }

        cdspUser = person "CDSP user" "Developer or user of use-cases" {
            -> cdsp "Creates or operates use-cases using"
        }

        southboundDataSources = softwareSystem "Southbound Data Sources" "e.g: real and virtual data feeder, simulator, gateway, ECUs" {
            tags "External Software System"

            -> cdsp "Sends timeseries data (VSS + other data models) to" "IoTDB API/SQL"
            cdsp -> southboundDataSources "Subscribes to data from"
        }

        bigDataCloud = softwareSystem "Big data" "Big data systems in the cloud" {
            tags "External Software System"

            -> cdsp "Co-operates with" "data-centric"
        }

        largeECUs = softwareSystem "Large ECUs" "e.g. HPC, zone and domain controllers" {
            tags "External Software System"

            -> cdsp "Co-operates with" "data-centric"
        }

        useCaseClients = softwareSystem "Use-case Clients" "Clients defining and manipulating data, e.g. functions and tooling, for use-cases" {
            tags "External Software System"

            -> cdsp "Makes data requests to" "e.g. VISS API, Native API, DB query language"
        }

        dbClients = softwareSystem "DB Clients" "Clients defining and manipulating data, in the VSS data model and other data models, using the database" {
            tags "External Software System"

            -> cdsp.timeseriesDataServer "Sends Data Definition and Data Manipulation requests to" "IoTDB API/SQL"
        }

        vissClients = softwareSystem "VISS Clients" "Clients manipulating VSS data using the VISS API " {
            tags "External Software System"

            -> cdsp.vissServer "Makes VISS get/set/sub requests for VSS data from" "VISS API"
        }

/* Original 'long form' relationship definitions from before they were dispersed to the components using 'short form'. 
   Left here temporarily for reference. Should be removed once new relationships agreed

        // Information Layer relationships
        cdsp.informationLayerServer.dbRouter -> onboardOffboardApplications "Consuming Information" "Websocket API"
        onboardOffboardApplications -> cdsp.informationLayerServer.dbRouter "Contributing Information" "Websocket API"
        cdsp.informationLayerServer.dbRouter -> cdsp.informationLayerServer.iotdbHandler "(De-)Auth / Read / Write / Subscribe" "Service Call"
        cdsp.informationLayerServer.iotdbHandler -> cdsp.timeseriesDataServer "Read / Write / (Subscribe)" "Node.js client"
        knowledgeEngineer -> cdsp.informationLayerServer.dbRouter "Provides semantic context for later conversion" "e.g. SHACL"
        cdsp.remotiveLabsBridge -> cdsp.informationLayerServer.dbRouter "Writes current value of data stream" "Websocket API"
        cdsp.informationLayerServer.dbRouter -> cdsp.knowledgeLayerServer.connector "Provides Information" "Websocket API"

        // Knowledge Layer relationships
        cdsp.knowledgeLayerServer.reasonerAdaptor -> cdsp.knowledgeLayerServer.rdfReasoner "Send triples, rules, queries" "HTTP"
        cdsp.knowledgeLayerServer.reasonerAdaptor -> cdsp.knowledgeLayerServer.rdfReasoner "Subscribes to new inferred triples" "HTTP"

        cdsp.knowledgeLayerServer.reasonerAdaptor -> cdsp.knowledgeLayerServer.connector "Inference result (rules)" "HTTP"
        cdsp.knowledgeLayerServer.reasonerAdaptor -> cdsp.knowledgeLayerServer.connector "Inference result (query)" "HTTP"
        cdsp.knowledgeLayerServer.connector -> cdsp.knowledgeLayerServer.reasonerAdaptor "Trigger Graph database interactions (ontologies, triples, rules)" "HTTP"
        knowledgeEngineer -> cdsp.knowledgeLayerServer.reasonerAdaptor "Initialize with ontologies and rules (representing business logic) and update" "HTTP"

        cdsp.knowledgeLayerServer.connector -> cdsp.knowledgeLayerServer.convertor "Sends tree-like data (json)" "HTTP"
        cdsp.knowledgeLayerServer.convertor -> cdsp.knowledgeLayerServer.connector "Sends Graph data (RDF)" "HTTP"
        cdsp.knowledgeLayerServer.convertor -> cdsp.knowledgeLayerServer.connector "Sends tree-like data (json)" "HTTP"
        cdsp.knowledgeLayerServer.connector -> cdsp.knowledgeLayerServer.convertor "Sends Graph data (RDF)" "HTTP"

        cdsp.knowledgeLayerServer.connector -> cdsp.informationLayerServer.dbRouter "Returns new Information" "Websocket API"
        cdsp.knowledgeLayerServer.connector -> onboardOffboardApplications "Consuming Knowledge" "HTTP SPARQL"

        // Graph RAG Server relationships
        aiAgents -> cdsp.graphRetrievalMcpServer.mcpTransport "Invokes Graph Retrieval Tool" "MCP Streamable HTTP"
        cdsp.graphRetrievalMcpServer.mcpTransport -> cdsp.graphRetrievalMcpServer.strategyLayer "Dispatches to selected strategy" "in-process"
        cdsp.graphRetrievalMcpServer.strategyLayer -> cdsp.graphRetrievalMcpServer.sharedServices "Delegates Retrieval Operations" "in-process"
        cdsp.graphRetrievalMcpServer.sharedServices -> cdsp.graphRetrievalMcpServer.vectorStoreAdapter "Semantic Node Search" "in-process"
        cdsp.graphRetrievalMcpServer.sharedServices -> cdsp.graphRetrievalMcpServer.knowledgeGraphAdapter "Graph Queries and Traversal" "in-process"
        cdsp.vectorStorageServer -> cdsp.graphRetrievalMcpServer.vectorStoreAdapter "Embedding Search" "HTTP REST"
        cdsp.graphRetrievalMcpServer.sharedServices -> cdsp.graphRetrievalMcpServer.informationLayerAdapter "Live Data and Timeseries" "in-process"
        cdsp.graphRetrievalMcpServer.informationLayerAdapter -> cdsp.informationLayerServer.dbRouter "Live and historical data" "Websocket API"
        cdsp.graphRetrievalMcpServer.knowledgeGraphAdapter -> cdsp.knowledgeLayerServer.connector "SPARQL QUERIES" "HTTP SPARQL"

        // RemotiveLabs relationships
        cdsp.remotiveLabsBridge -> remotiveLabsPlatform "Subscribes to signals" "Remotive API"
        remotiveLabsPlatform -> cdsp.remotiveLabsBridge "Sends signal data timeseries" "Remotive API"
        cdsp.remotiveLabsBridge -> cdsp.timeseriesDataServer "Writes timeseries data stream" "IoTDB API/SQL"
*/
    }

    views {
        systemContext cdsp "CDSP-SystemContext" {
            include *
            exclude knowledgeEngineer remotiveLabsPlatform dbClients vissClients onboardOffboardApplications aiAgents
            autolayout tb
#            title "CDSP System Context"
            description "CDSP and its users and external software systems."
        }

        container cdsp "CDSP-Containers" {
            include *
            autolayout lr
            #title "CDSP Container View"
            description "The principal CDSP servers, stores, bridge and external dependencies."
        }

        component cdsp.informationLayerServer "IL-Components" {
            description "The component diagram for the Information Layer Container"
            include *
#            exclude "cdsp.remotiveLabsBridge -> cdsp.timeseriesDataServer"
            autolayout lr
#            title "CDSP Information Layer Server Components"
        }

        component cdsp.knowledgeLayerServer "KL-Components" {
            description "The component diagram for the Knowledge Layer Container"

            include *
            autolayout lr
#            title "CDSP Knowledge Layer Server Components"
        }

        component cdsp.graphRetrievalMcpServer "Graph-RAG-Components" {
            description "The component diagram for the Graph Retrieval MCP Server"

            include *
            autolayout lr
#            title "Graph Retrieval MCP Server Components"
        }

        styles {
            element "Person" {
                shape person
                background #08427b
                color #ffffff
            }
            element "Software System" {
                background #1168bd
                color #ffffff
            }
            element "External Software System" {
                background #8c8496
                color #ffffff
            }
            element "Container" {
                background #23a2d9
                color #ffffff
            }
            element "Component" {
                background #c2c9eb
                color #222222
                stroke #000000
            }
            element "Database" {
                shape cylinder
            }
            relationship "Reconstructed" {
                dashed true
                color #b36b00
            }
        }

    }

    configuration {
        scope softwaresystem
    }
}
