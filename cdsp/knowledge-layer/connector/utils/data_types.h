#ifndef DATA_TYPES_H
#define DATA_TYPES_H

#include <chrono>
#include <map>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "helper.h"

/**
 * @brief Enum class for the supported inference engine types
 */
enum class InferenceEngineType {
    RDFOX,
};

/**
 * @brief Enum class for the message types
 */
enum class MessageType {
    DATA,
    GET,
    SET,
    SUBSCRIBE,
    UNSUBSCRIBE,
};

/**
 * @brief Enum class for the supported schema types
 */
enum class SchemaType {
    VEHICLE,
    DEFAULT,
};

/**
 * @brief Enum class for the reasoner syntax types
 */
enum class ReasonerSyntaxType {
    TURTLE,    ///< Terse triples http://www.w3.org/TR/turtle
    NTRIPLES,  ///< Line-based triples http://www.w3.org/TR/n-triples/
    NQUADS,    ///< Line-based quads http://www.w3.org/TR/n-quads/
    TRIG,      ///< Terse quads http://www.w3.org/TR/trig/
};

/**
 * @brief Enum class for the reasoning query language types
 */
enum class QueryLanguageType {
    SPARQL,
};

/**
 * @brief Enum class for the reasoning rule language types
 */
enum class RuleLanguageType {
    DATALOG,
};

/**
 * @brief Enum class for the data query accept types
 */
enum class DataQueryAcceptType {
    TEXT_TSV,
    TEXT_CSV,
    SPARQL_JSON,
    SPARQL_XML,
};

/**
 * @brief Enum class for the supported message structure formats
 */
enum class MessageStructureFormat {
    FLAT,
    NESTED,
    LEAF,
};

/**
 * @brief Configuration structure for the websocket servers
 */
struct WSServerData {
    std::string host;
    std::string port;
    std::string target;
};

/**
 * @brief Configuration structure for the reasoner servers
 */
struct ReasonerServerData {
    std::string host;
    std::string port;
    std::string auth_base64;
    std::optional<std::string> data_store_name;
};

/**
 * @brief Configuration structure for the WebSocket client
 */
struct SystemConfig {
    std::string uuid;
    WSServerData websocket_server;
    ReasonerServerData reasoner_server;
};

/**
 * @brief Converts a MessageType enum to its corresponding string representation.
 *
 * @param type The MessageType enum value to be converted.
 * @return A std::string representing the name of the MessageType.
 *
 * @throws std::runtime_error if the input MessageType is not supported.
 */
std::string messageTypeToString(const MessageType& type);

/**
 * @brief Converts a reasoner output format string to an ReasonerSyntaxType.
 *
 * This function takes a string representing a reasoner output format and
 * converts it to the corresponding ReasonerSyntaxType. The input string should
 * match one of the predefined reasoner output formats.
 *
 * @param type A string representing the reasoner output format.
 * @return The corresponding ReasonerSyntaxType for the given reasoner output format.
 *
 * @throws std::runtime_error if the input reasoner output format is not supported.
 */
ReasonerSyntaxType reasonerOutputFormatToReasonerSyntaxType(const std::string& type);

/**
 * @brief Converts a string file extension to an ReasonerSyntaxType.
 *
 * This function takes a string representing a file extension and converts it to the corresponding
 * ReasonerSyntaxType. The input string should match one of the predefined RDF syntax extensions.
 *
 * @param extension A string representing the file extension.
 * @return The corresponding ReasonerSyntaxType for the given file extension.
 *
 * @throws std::runtime_error if the input file extension is not supported.
 */
ReasonerSyntaxType fileExtensionToReasonerSyntaxType(const std::string& extension);

/**
 * @brief Converts a ReasonerSyntaxType to its corresponding file extension.
 *
 * This function takes a ReasonerSyntaxType enumeration value and returns the
 * corresponding file extension as a string. This is useful for determining the
 * file extension associated with a particular RDF syntax.
 *
 * @param type The ReasonerSyntaxType to be converted.
 * @return A string representing the file extension associated with the given RDF syntax type.
 */
std::string reasonerSyntaxTypeToFileExtension(const ReasonerSyntaxType& type);

/**
 * @brief Converts an RDF syntax type to its corresponding content type string.
 *
 * This function takes an ReasonerSyntaxType enumeration value and returns the
 * corresponding content type for the REST API as a string. This is useful for determining
 * the MIME type associated with a particular RDF syntax.
 *
 * @param type The RDF syntax type to be converted.
 * @return A string representing the content type associated with the given RDF syntax type.
 */
std::string reasonerSyntaxTypeToContentType(const ReasonerSyntaxType& type);

/**
 * @brief Converts a file extension to its corresponding QueryLanguageType.
 *
 * This function takes a string representing a file extension and maps it to
 * the appropriate QueryLanguageType. This is useful for determining the query
 * language associated with a particular file type based on its extension.
 *
 * @param extension A string representing the file extension.
 * @return The corresponding QueryLanguageType for the given file extension.
 */
QueryLanguageType fileExtensionToQueryLanguageType(const std::string& extension);

/**
 * @brief Converts a QueryLanguageType to its corresponding content type string.
 *
 * This function takes a QueryLanguageType enumeration value and returns the
 * corresponding content type as a string. This is useful for determining
 * the MIME type associated with a particular query language.
 *
 * @param type The QueryLanguageType to be converted.
 * @return A string representing the content type associated with the given QueryLanguageType.
 */
std::string queryLanguageTypeToContentType(const QueryLanguageType& type);

/**
 * @brief Converts a file extension to its corresponding RuleLanguageType.
 *
 * This function takes a file extension as input and returns the associated
 * RuleLanguageType. It is used to map file extensions to their respective
 * rule language types for processing.
 *
 * @param extension The file extension as a string.
 * @return RuleLanguageType The corresponding RuleLanguageType for the given extension.
 */
RuleLanguageType fileExtensionToRuleLanguageType(const std::string& extension);

/**
 * @brief Converts a RuleLanguageType to its corresponding content type string.
 *
 * This function takes a RuleLanguageType enumeration value and returns the
 * corresponding content type as a std::string. The mapping between the
 * RuleLanguageType and the content type string is predefined within the function.
 *
 * @param type The RuleLanguageType enumeration value to be converted.
 * @return A std::string representing the content type corresponding to the given RuleLanguageType.
 */
std::string ruleLanguageTypeToContentType(const RuleLanguageType& type);

/**
 * @brief Converts a QueryAcceptType enum to its corresponding string representation.
 *
 * @param type The DataQueryAcceptType enum value to be converted.
 * @return A std::string representing the name of the DataQueryAcceptType.
 *
 * @throws std::runtime_error if the input DataQueryAcceptType is not supported.
 */
std::string queryAcceptTypeToString(const DataQueryAcceptType& type);

/**
 * @brief Converts a SchemaType enum value to its corresponding string representation.
 *
 * This function takes a SchemaType enum value as input and returns a string that
 * represents the name of the schema type.
 *
 * @param type The SchemaType enum value to be converted to a string.
 * @param capitalizeFirstLetter Boolean flag to capitalize the first letter of the string if true.
 * @return A std::string representing the name of the schema type.
 *
 * @throws std::runtime_error if the input SchemaType is not supported.
 */
std::string schemaTypeToString(const SchemaType& type, bool capitalizeFirstLetter = false);

/**
 * @brief Converts a string to a SchemaType enum value.
 *
 * This function takes a string as input and converts it to the corresponding
 * SchemaType enum value. The input string should match one of the predefined
 * schema types.
 *
 * @param type A string representing the schema type.
 * @return The corresponding SchemaType enum value for the given string.
 *
 * @throws std::runtime_error if the input string does not match any of the predefined schema
 * definitions.
 */
SchemaType stringToSchemaType(const std::string& type);

/**
 * @brief Converts a InferenceEngineType enum value to its corresponding string representation.
 *
 * This function takes a InferenceEngineType enum value as input and returns a string that
 * represents the name of the InferenceEngineType.
 *
 * @param type The InferenceEngineType enum value to be converted.
 * @return A std::string representing the name of the InferenceEngineType.
 */
InferenceEngineType stringToInferenceEngineType(const std::string& type);

/**
 * @brief Converts a InferenceEngineType enum value to string.
 *
 * This function takes an InferenceEngineType enum value as input and returns a string that
 * represents the name of the InferenceEngineType.
 *
 * @param type The InferenceEngineType enum value to be converted.
 * @return A std::string representing the name of the InferenceEngineType.
 *
 * @throws std::runtime_error if the input InferenceEngineType is not supported.
 */
std::string inferenceEngineTypeToString(const InferenceEngineType& type);

/**
 * @brief Converts a structure message format enum to its corresponding string representation.
 *
 * @param type The MessageStructureFormat enum value to be converted.
 * @return A std::string representing the name of the MessageStructureFormat.
 *
 * @throws std::runtime_error if the input MessageStructureFormat is not supported.
 */
std::string MessageStructureFormatToString(const MessageStructureFormat& type);

inline std::string messageTypeToString(const MessageType& type) {
    switch (type) {
        case MessageType::DATA:
            return "data";
        case MessageType::GET:
            return "get";
        case MessageType::SET:
            return "set";
        case MessageType::SUBSCRIBE:
            return "subscribe";
        case MessageType::UNSUBSCRIBE:
            return "unsubscribe";
        default:
            throw std::runtime_error("Unsupported message type");
    }
};

inline ReasonerSyntaxType reasonerOutputFormatToReasonerSyntaxType(const std::string& type) {
    std::string lowerCaseType = Helper::toLowerCase(type);
    if (lowerCaseType == "turtle") {
        return ReasonerSyntaxType::TURTLE;
    } else if (lowerCaseType == "ntriples") {
        return ReasonerSyntaxType::NTRIPLES;
    } else if (lowerCaseType == "nquads") {
        return ReasonerSyntaxType::NQUADS;
    } else if (lowerCaseType == "trig") {
        return ReasonerSyntaxType::TRIG;
    } else {
        throw std::invalid_argument("Unsupported output format: " + type);
    }
}

inline ReasonerSyntaxType fileExtensionToReasonerSyntaxType(const std::string& extension) {
    std::string lowerCaseExtension = Helper::toLowerCase(extension);
    if (lowerCaseExtension == ".ttl") {
        return ReasonerSyntaxType::TURTLE;
    } else if (lowerCaseExtension == ".nq") {
        return ReasonerSyntaxType::NQUADS;
    } else if (lowerCaseExtension == ".nt") {
        return ReasonerSyntaxType::NTRIPLES;
    } else if (lowerCaseExtension == ".trig") {
        return ReasonerSyntaxType::TRIG;
    } else {
        throw std::invalid_argument("Unsupported syntax type file extension: " + extension);
    }
}

inline std::string reasonerSyntaxTypeToFileExtension(const ReasonerSyntaxType& type) {
    switch (type) {
        case ReasonerSyntaxType::TURTLE:
            return ".ttl";
        case ReasonerSyntaxType::NQUADS:
            return ".nq";
        case ReasonerSyntaxType::NTRIPLES:
            return ".nt";
        case ReasonerSyntaxType::TRIG:
            return ".trig";
        default:
            throw std::invalid_argument("Unsupported syntax type");
    }
}

inline std::string reasonerSyntaxTypeToContentType(const ReasonerSyntaxType& type) {
    switch (type) {
        case ReasonerSyntaxType::TURTLE:
            return "text/turtle";
        case ReasonerSyntaxType::NTRIPLES:
            return "application/n-triples";
        case ReasonerSyntaxType::NQUADS:
            return "application/n-quads";
        case ReasonerSyntaxType::TRIG:
            return "application/trig";
        default:
            throw std::invalid_argument("Unsupported RDF syntax type");
    }
}

inline QueryLanguageType fileExtensionToQueryLanguageType(const std::string& extension) {
    std::string lowerCaseExtension = Helper::toLowerCase(extension);
    if (lowerCaseExtension == ".rq") {
        return QueryLanguageType::SPARQL;
    } else {
        throw std::invalid_argument("Unsupported query file extension: " + extension);
    }
}

inline std::string queryLanguageTypeToContentType(const QueryLanguageType& type) {
    switch (type) {
        case QueryLanguageType::SPARQL:
            return "application/sparql-query";
        default:
            throw std::invalid_argument("Unsupported query language type");
    }
}

inline RuleLanguageType fileExtensionToRuleLanguageType(const std::string& extension) {
    std::string lowerCaseExtension = Helper::toLowerCase(extension);
    if (lowerCaseExtension == ".dlog") {
        return RuleLanguageType::DATALOG;
    } else {
        throw std::invalid_argument("Unsupported rule file extension: " + extension);
    }
}

inline std::string ruleLanguageTypeToContentType(const RuleLanguageType& type) {
    switch (type) {
        case RuleLanguageType::DATALOG:
            return "application/x.datalog";
        default:
            throw std::invalid_argument("Unsupported rule language type");
    }
}

inline std::string queryAcceptTypeToString(const DataQueryAcceptType& type) {
    switch (type) {
        case DataQueryAcceptType::TEXT_CSV:
            return "text/csv";
        case DataQueryAcceptType::TEXT_TSV:
            return "text/tab-separated-values";
        case DataQueryAcceptType::SPARQL_JSON:
            return "application/sparql-results+json";
        case DataQueryAcceptType::SPARQL_XML:
            return "application/sparql-results+xml";
        default:
            throw std::invalid_argument("Unsupported query accept type");
    }
};

inline std::string schemaTypeToString(const SchemaType& type, bool capitalizeFirstLetter) {
    std::string schema;
    switch (type) {
        case SchemaType::VEHICLE:
            schema = "vehicle";
            break;
        default:
            throw std::invalid_argument("Unsupported schema type");
    }
    if (capitalizeFirstLetter) {
        schema[0] = toupper(schema[0]);
    }
    return schema;
};

inline SchemaType stringToSchemaType(const std::string& type) {
    std::string lowerCaseType = Helper::toLowerCase(type);
    if (lowerCaseType == "vehicle") {
        return SchemaType::VEHICLE;
    } else {
        throw std::invalid_argument("Unsupported schema type: " + type);
    }
}

inline std::string MessageStructureFormatToString(const MessageStructureFormat& type) {
    switch (type) {
        case MessageStructureFormat::FLAT:
            return "flat";
        case MessageStructureFormat::NESTED:
            return "nested";
        case MessageStructureFormat::LEAF:
            return "leaf";
        default:
            throw std::invalid_argument("Unsupported message structure format");
    }
}

inline InferenceEngineType stringToInferenceEngineType(const std::string& type) {
    std::string lowerCaseType = Helper::toLowerCase(type);
    if (lowerCaseType == "rdfox") {
        return InferenceEngineType::RDFOX;
    } else {
        throw std::invalid_argument("Unsupported inference engine string type: " + type);
    }
}

inline std::string inferenceEngineTypeToString(const InferenceEngineType& type) {
    switch (type) {
        case InferenceEngineType::RDFOX:
            return "RDFox";
        default:
            throw std::invalid_argument("Unsupported inference engine type");
    }
}

#endif  // DATA_TYPES_H