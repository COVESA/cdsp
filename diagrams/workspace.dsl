workspace "CDSP Knowledge, Information and Graph Retrieval Architecture" "Converted from CDSP-C4-Model-Graph-Retrieval-Server-Component-diagram.drawio.xml (diagram v0.4)" {

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

        cdsp = softwareSystem "COVESA Central Data Service Playground (CDSP)" "Central data service playground providing information, knowledge, graph retrieval and supporting data services." {

            informationLayerServer = container "CDSP Information Layer Server" "Provides access to information-layer data and abstracts the underlying time-series data store." "Node.js" {
                dbRouter = component "DB Router" "Provides information layer data in tree-like format. Abstracts data store details" "Node.JS Websocket Server"
                iotdbHandler = component "IotDB Handler" "Provides information layer data in tree-like format. Abstracts data store details" "Node.JS Websocket Server"
            }

            timeseriesDataServer = container "Timeseries Data Server" "Serves timeseries data in VSS and other data-models" "Apache IoTDB" {
                tags "Database"
            }

            knowledgeLayerServer = container "CDSP Knowledge Layer Server" "Converts information-layer data to graph data and provides knowledge-graph reasoning and query capabilities." "C++" {
                convertor = component "Convertor" "Converts tree-like data (json) into graph data (RDF) and vice versa" "C++"
                connector = component "Connector" "Connects Knowledge Layer to Information Layer and manage data tasks within KL" "Websocket Client"
                reasonerAdaptor = component "Reasoner-Adaptor" "Interacts with Graph database and reasoner by API (e.g. allows import of RDF data and rules and execute queries)" "C++"
                rdfReasoner = component "RDF Reasoner Service and Knowledge Graph" "Stores RDF data, executes rules, and evaluates queries." "RDFox or RDF4J"
            }

            graphRetrievalMcpServer = container "(WIP) Graph Retrieval MCP Server" "Provides graph retrieval strategies to AI agents through MCP." "TypeScript" {
                mcpTransport = component "MCP Transport" "Exposes all strategies as MCP tools. Handles Session and Auth" "Streamable HTTP"
                strategyLayer = component "Strategy Layer" "seed_expand, local_fanout, path_find, timeseries_aware + extensible" "TypeScript"
                sharedServices = component "Shared Services" "Semantic Search, Graph Traversal, Live Data, Temporal Analysis" "TypeScript"
                knowledgeGraphAdapter = component "Knowledge Graph Adapter" "query (sparql), traverse (seeds), pathFind(from, to), describe(iri)" "Interface"
                vectorStoreAdapter = component "Vector Store Adapter" "search(query, n), filter(query, metadata)" "Interface"
                informationLayerAdapter = component "Information Layer Adapter" "get(path, instance), subscribe(path), timeseries(path, range)" "Interface"
            }

            vectorStorageServer = container "Vector Storage Server" "Ontology Node Embeddings" "e.g. Chroma" {
                tags "Database"
            }

            remotiveLabsBridge = container "RemotiveLabs Bridge" "Streams subscribed timeseries data from RemotiveLabs virtual signal platform to Playground data store" "Python"
        }

        // Information Layer relationships
        cdsp.informationLayerServer.dbRouter -> onboardOffboardApplications "Consuming Information" "Websocket API"
        onboardOffboardApplications -> cdsp.informationLayerServer.dbRouter "Contributing Information" "Websocket API"
        cdsp.informationLayerServer.dbRouter -> cdsp.informationLayerServer.iotdbHandler "(De-)Auth / Read / Write / Subscribe" "Service Call"
        cdsp.informationLayerServer.iotdbHandler -> cdsp.timeseriesDataServer "Read / Write / (Subscribe)" "Node.js client"
        knowledgeEngineer -> cdsp.informationLayerServer.dbRouter "Provides semantic context for later conversion" "e.g. SHACL"
        cdsp.remotiveLabsBridge -> cdsp.informationLayerServer.dbRouter "Writes current value of data stream" "Websocket API"

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

        onboardOffboardApplications -> cdsp.knowledgeLayerServer.connector "Consuming Knowledge" "HTTP SPARQL"

        // Graph RAG Server
        aiAgents -> cdsp.graphRetrievalMcpServer.mcpTransport "Invokes Graph Retrieval Tool" "MCP Streamable HTTP"
        cdsp.graphRetrievalMcpServer.mcpTransport -> cdsp.graphRetrievalMcpServer.strategyLayer "Dispatches to selected strategy" "in-process"
        cdsp.graphRetrievalMcpServer.strategyLayer -> cdsp.graphRetrievalMcpServer.sharedServices "Delegates Retrieval Operations" "in-process"
        cdsp.graphRetrievalMcpServer.sharedServices -> cdsp.graphRetrievalMcpServer.vectorStoreAdapter "Semantic Node Search" "in-process"
        cdsp.graphRetrievalMcpServer.sharedServices -> cdsp.graphRetrievalMcpServer.knowledgeGraphAdapter "Graph Queries and Traversal" "in-process"
        cdsp.vectorStorageServer -> cdsp.graphRetrievalMcpServer.vectorStoreAdapter "Embedding Search" "HTTP REST"
        cdsp.graphRetrievalMcpServer.sharedServices -> cdsp.graphRetrievalMcpServer.informationLayerAdapter "Live Data and Timeseries" "in-process"
        cdsp.graphRetrievalMcpServer.informationLayerAdapter -> cdsp.informationLayerServer.dbRouter "Live and historical data" "Websocket API"

        // RemotiveLabs
        cdsp.remotiveLabsBridge -> remotiveLabsPlatform "Subscribes to signals" "Remotive API"
        remotiveLabsPlatform -> cdsp.remotiveLabsBridge "Sends signal data timeseries" "Remotive API"
        cdsp.remotiveLabsBridge -> cdsp.timeseriesDataServer "Writes timeseries data stream" "IoTDB API/SQL"



/* Connections created by AI for reference
        cdsp.informationLayerServer.dbRouter -> cdsp.informationLayerServer.iotdbHandler "(De-)Auth / Read / Write / Subscribe" "Service Call"
        cdsp.informationLayerServer.iotdbHandler -> cdsp.timeseriesDataServer "Read / Write / (Subscribe)" "Thrift"

        cdsp.knowledgeLayerServer.connector -> cdsp.graphRetrievalMcpServer.knowledgeGraphAdapter "SPARQL QUERIES" "HTTP SPARQL"
        cdsp.informationLayerServer.dbRouter -> cdsp.graphRetrievalMcpServer.informationLayerAdapter "Live and historical data" "Websocket API"

        knowledgeEngineer -> cdsp.knowledgeLayerServer.reasonerAdaptor "Initialize with ontologies and rules (representing business logic) and update" "HTTP"
        knowledgeEngineer -> cdsp.informationLayerServer.dbRouter "Provides semantic context for later conversion" "e.g. SHACL"

        cdsp.remotiveLabsBridge -> remotiveLabsPlatform "Subscribes to signals" "Remotive API"
        remotiveLabsPlatform -> cdsp.remotiveLabsBridge "Sends signal data timeseries" "Remotive API"
        cdsp.remotiveLabsBridge -> cdsp.timeseriesDataServer "Writes timeseries data stream" "IoTDB API/SQL"

        cdsp.graphRetrievalMcpServer.vectorStoreAdapter -> cdsp.graphRetrievalMcpServer.sharedServices "Semantic Node Search" "in-process"
        cdsp.graphRetrievalMcpServer.knowledgeGraphAdapter -> cdsp.graphRetrievalMcpServer.sharedServices "Graph Queries and Traversal" "in-process"
        cdsp.graphRetrievalMcpServer.sharedServices -> cdsp.graphRetrievalMcpServer.strategyLayer "Delegates Retrieval Operations" "in-process"
        cdsp.graphRetrievalMcpServer.strategyLayer -> cdsp.graphRetrievalMcpServer.mcpTransport "Dispatches to selected strategy" "in-process"
        cdsp.vectorStorageServer -> cdsp.graphRetrievalMcpServer.vectorStoreAdapter "Embedding Search" "HTTP REST"
        cdsp.graphRetrievalMcpServer.informationLayerAdapter -> cdsp.graphRetrievalMcpServer.sharedServices "Live Data and Timeseries" "in-process"
        cdsp.graphRetrievalMcpServer.mcpTransport -> aiAgents "Invokes Graph Retrieval Tool" "MCP Streamable HTTP"

        // Relationships reconstructed from geometry-only connectors in the draw.io XML
        onboardOffboardApplications -> cdsp.knowledgeLayerServer.connector "Contributing Information" "Websocket API" {
            tags "Reconstructed"
        }
        onboardOffboardApplications -> cdsp.knowledgeLayerServer.connector "Consuming Knowledge" "HTTP SPARQL" {
            tags "Reconstructed"
        }
        cdsp.informationLayerServer.dbRouter -> cdsp.knowledgeLayerServer.connector "Provides Information" "Websocket API" {
            tags "Reconstructed"
        }
        cdsp.knowledgeLayerServer.connector -> cdsp.informationLayerServer.dbRouter "Returns new Information" "Websocket API" {
            tags "Reconstructed"
        }
        cdsp.knowledgeLayerServer.connector -> cdsp.knowledgeLayerServer.convertor "Sends tree-like data (json)" "HTTP" {
            tags "Reconstructed"
        }
        cdsp.knowledgeLayerServer.convertor -> cdsp.knowledgeLayerServer.connector "Sends Graph data (RDF)" "HTTP" {
            tags "Reconstructed"
        }
        cdsp.knowledgeLayerServer.connector -> cdsp.knowledgeLayerServer.reasonerAdaptor "Trigger Graph database interactions (ontologies, triples, rules)" "HTTP" {
            tags "Reconstructed"
        }
        cdsp.knowledgeLayerServer.reasonerAdaptor -> cdsp.knowledgeLayerServer.connector "Inference result (rules)" "HTTP" {
            tags "Reconstructed"
        }
        cdsp.knowledgeLayerServer.reasonerAdaptor -> cdsp.knowledgeLayerServer.connector "Inference result (query)" "HTTP" {
            tags "Reconstructed"
        }
        cdsp.remotiveLabsBridge -> cdsp.informationLayerServer.dbRouter "Writes current value of data stream" "Websocket API" {
            tags "Reconstructed"
        }
*/
    }

    views {
        systemContext cdsp "CDSP-SystemContext" {
            include *
            autolayout lr
            title "CDSP System Context"
            description "CDSP and its users and external software systems."
        }

        container cdsp "CDSP-Containers" {
            include *
            autolayout lr
            title "CDSP Container View"
            description "The principal CDSP servers, stores, bridge and external dependencies."
        }

        component cdsp.informationLayerServer "IL-Components" {
            description "The component diagram for the Information Layer Container"
            include *
/*
            include cdsp.timeseriesDataServer
            include cdsp.knowledgeLayerServer.connector
            include cdsp.graphRetrievalMcpServer.informationLayerAdapter
            include onboardOffboardApplications
            include knowledgeEngineer
            include cdsp.remotiveLabsBridge
*/            
            autolayout lr
#            title "CDSP Information Layer Server Components"
        }

        component cdsp.knowledgeLayerServer "KL-Components" {
            description "The component diagram for the Knowledge Layer Container"

            include *
/*
            include cdsp.informationLayerServer.dbRouter
            include cdsp.graphRetrievalMcpServer.knowledgeGraphAdapter
            include onboardOffboardApplications
            include knowledgeEngineer
*/
            autolayout lr
#            title "CDSP Knowledge Layer Server Components"
        }

        component cdsp.graphRetrievalMcpServer "Graph-RAG-Components" {
            description "The component diagram for the Graph Retrieval MCP Server"

            include *
/*            
            include aiAgents
            include cdsp.vectorStorageServer
            include cdsp.knowledgeLayerServer.connector
            include cdsp.informationLayerServer.dbRouter
            autolayout tb
*/
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
