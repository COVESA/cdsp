#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "mock_rdfox_adapter.h"
#include "random_utils.h"
#include "request_expectations.h"

class RDFoxAdapterTest : public ::testing::Test {
    // NOLINTBEGIN(cppcoreguidelines-non-private-member-variables-in-classes)
   protected:
    const std::string DATASTORE = "ds-test";
    const std::string TTL_DATA = "@prefix : <http://example.org/> . :test a :Entity .";
    const std::string DATALOG_RULES = "[?x a :SomeClass] :- [?x :someProperty :someValue] .";
    const std::string SPARQL_QUERY = "SELECT ?s WHERE { ?s ?p ?o . }";
    const ReasonerServerData server_data_{"localhost", "8080", "auth", "", DATASTORE};

    std::shared_ptr<MockRDFoxAdapter> adapter_;

    // NOLINTEND(cppcoreguidelines-non-private-member-variables-in-classes)
    void SetUp() override { adapter_ = std::make_shared<MockRDFoxAdapter>(server_data_); }

    ExpectedRequest listDatastoresRequest(const std::string& datastores) {
        return ExpectedRequest(http::verb::get, "/datastores")
            .withAcceptType("text/csv; charset=UTF-8")
            .respondsWith(datastores);
    }

    ExpectedRequest createDatastoreRequest() {
        return ExpectedRequest(http::verb::post, "/datastores/" + DATASTORE)
            .withContentType("application/json");
    }

    ExpectedRequest loadContentRequest(const std::string& content_type, const std::string& body) {
        return ExpectedRequest(http::verb::post, "/datastores/" + DATASTORE + "/content")
            .withContentType(content_type)
            .withBody(body);
    }

    ExpectedRequest loadDataRequest() {
        return loadContentRequest(reasonerSyntaxTypeToContentType(ReasonerSyntaxType::TURTLE),
                                  TTL_DATA);
    }

    ExpectedRequest queryRequest() {
        return ExpectedRequest(http::verb::post, "/datastores/" + DATASTORE + "/sparql")
            .withContentType("application/sparql-query")
            .withBody(SPARQL_QUERY)
            .withAcceptType(queryAcceptTypeToString(DataQueryAcceptType::TEXT_TSV));
    }

    ExpectedRequest createConnectionRequest() {
        return ExpectedRequest(http::verb::post, "/datastores/" + DATASTORE + "/connections")
            .withContentType("application/json");
    }

    ExpectedRequest createCursorRequest(const std::string& connection_id,
                                        const std::string& auth_token, const std::string& query) {
        return ExpectedRequest(http::verb::post, "/datastores/" + DATASTORE + "/connections/" +
                                                     connection_id + "/cursors")
            .withContentType("application/sparql-query")
            .withAuthorization(auth_token)
            .withBody(query);
    }

    ExpectedRequest advanceCursorRequest(const std::string& connection_id,
                                         const std::string& auth_token,
                                         const std::string& cursor_id, const std::string& operation,
                                         int limit) {
        return ExpectedRequest(http::verb::patch, "/datastores/" + DATASTORE + "/connections/" +
                                                      connection_id + "/cursors/" + cursor_id +
                                                      "?operation=" + operation +
                                                      "&limit=" + std::to_string(limit))
            .withAuthorization(auth_token)
            .withAcceptType(queryAcceptTypeToString(DataQueryAcceptType::SPARQL_JSON));
    }

    ExpectedRequest deleteCursorRequest(const std::string& connection_id,
                                        const std::string& cursor_id) {
        return ExpectedRequest(http::verb::delete_, "/datastores/" + DATASTORE + "/connections/" +
                                                        connection_id + "/cursors/" + cursor_id);
    }

    static std::string randomOperation() {
        return RandomUtils::generateRandomInt(0, 1) == 0 ? "open" : "advance";
    }
};

// Unit tests for RDFoxAdapter to verify success regular operations

/**
 * @brief Test fixture for RDFoxAdapter to check the existence of a datastore.
 *
 * This test verifies that the RDFoxAdapter correctly constructs a request
 * to check the existence of a datastore and receives the expected response.
 */
TEST_F(RDFoxAdapterTest, CheckExistentDatastore) {
    expectRequests(*adapter_, {listDatastoresRequest("Name\r\nds-test\r\n")});

    EXPECT_TRUE(adapter_->checkDataStore());
}

/**
 * @brief A datastore whose name only starts with the configured name does not count.
 */
TEST_F(RDFoxAdapterTest, CheckDatastoreMatchesTheExactName) {
    expectRequests(*adapter_, {listDatastoresRequest("Name\r\nds-test-2\r\n")});

    EXPECT_FALSE(adapter_->checkDataStore());
}

/**
 * @brief Unit test for RDFoxAdapter to verify the initialization process
 * creates a datastore.
 */
TEST_F(RDFoxAdapterTest, InitializationCreatesDatastore) {
    expectRequests(*adapter_, {listDatastoresRequest("Name\r\n"), createDatastoreRequest()});

    EXPECT_NO_THROW(adapter_->initialize());
}

/**
 * @brief Unit test for RDFoxAdapter to verify the behavior during
 * initialization when a datastore already exists.
 */
TEST_F(RDFoxAdapterTest, InitializationWithExistingDatastore) {
    expectRequests(*adapter_, {listDatastoresRequest("Name\r\nds-test\r\n")});

    EXPECT_NO_THROW(adapter_->initialize());
}

/**
 * @brief Unit test for RDFoxAdapter to verify the behavior when loading data
 * into a datastore.
 */
TEST_F(RDFoxAdapterTest, LoadDataSuccess) {
    expectRequests(*adapter_, {loadDataRequest()});

    EXPECT_TRUE(
        adapter_->loadData(TTL_DATA, reasonerSyntaxTypeToContentType(ReasonerSyntaxType::TURTLE)));
}

/**
 * @brief Unit test for RDFoxAdapter to verify successful data querying.
 */
TEST_F(RDFoxAdapterTest, QueryDataSuccess) {
    const std::string mock_response = "<http://example.org/test>";
    expectRequests(*adapter_, {queryRequest().respondsWith(mock_response)});

    EXPECT_EQ(
        adapter_->queryData(SPARQL_QUERY, QueryLanguageType::SPARQL, DataQueryAcceptType::TEXT_TSV),
        mock_response);
}

/**
 * @brief Unit test for RDFoxAdapter to verify successful deletion of a
 * datastore.
 */
TEST_F(RDFoxAdapterTest, DeleteDataStoreSuccess) {
    expectRequests(*adapter_, {listDatastoresRequest("Name\r\nds-test\r\n"),
                               ExpectedRequest(http::verb::delete_, "/datastores/" + DATASTORE)});

    EXPECT_TRUE(adapter_->deleteDataStore());
}

/**
 * @brief Unit test for RDFoxAdapter to verify the behavior when attempting to
 * delete a nonexistent datastore.
 */
TEST_F(RDFoxAdapterTest, DeleteNonexistentDataStoreSuccess) {
    expectRequests(*adapter_, {listDatastoresRequest("Name\r\n")});

    EXPECT_TRUE(adapter_->deleteDataStore());
}

// Unit tests for RDFoxAdapter to verify success cursor-related operations

/**
 * @brief Unit test for RDFoxAdapter to verify successful connection creation.
 */
TEST_F(RDFoxAdapterTest, CreateConnectionSuccess) {
    const std::string random_connection_id =
        std::to_string(RandomUtils::generateRandomInt(0, 1000));
    const std::string random_auth = RandomUtils::generateRandomString(8);
    expectRequests(*adapter_,
                   {createConnectionRequest().respondsWithHeaders(
                       {{"Location", "/datastores/datastore/connections/" + random_connection_id},
                        {"RDFox-Authentication-Token", random_auth}})});

    auto connection = adapter_->createConnection();

    EXPECT_EQ(connection.first, random_connection_id);
    EXPECT_EQ(connection.second, "RDFox " + random_auth);
}

/**
 * @brief Unit test for RDFoxAdapter to verify successful cursor creation.
 */
TEST_F(RDFoxAdapterTest, CreateCursorSuccess) {
    const std::string connection_id = RandomUtils::generateRandomString(8);
    const std::string auth_token = RandomUtils::generateRandomString(8);
    const std::string valid_query = RandomUtils::generateRandomString(20);
    const std::string cursor_id = RandomUtils::generateRandomString(8);
    const std::string location_header =
        "/datastores/" + DATASTORE + "/connections/" + connection_id + "/cursors/" + cursor_id;
    expectRequests(*adapter_, {createCursorRequest(connection_id, auth_token, valid_query)
                                   .respondsWithHeaders({{"Location", location_header}})});

    std::string created_cursor_id;
    ASSERT_NO_THROW(created_cursor_id =
                        adapter_->createCursor(connection_id, auth_token, valid_query));
    EXPECT_EQ(created_cursor_id, cursor_id);
}

/**
 * @brief Unit test for RDFoxAdapter to verify successful cursor advance.
 */
TEST_F(RDFoxAdapterTest, AdvanceCursorSuccess) {
    const std::string connection_id = RandomUtils::generateRandomString(8);
    const std::string auth_token = RandomUtils::generateRandomString(8);
    const std::string cursor_id = RandomUtils::generateRandomString(8);
    const std::string response_body = RandomUtils::generateRandomString(20);
    const std::string operation = randomOperation();
    const int limit = RandomUtils::generateRandomInt(1, 100);
    expectRequests(*adapter_,
                   {advanceCursorRequest(connection_id, auth_token, cursor_id, operation, limit)
                        .respondsWith(response_body)});

    std::string response;
    EXPECT_TRUE(adapter_->advanceCursor(connection_id, auth_token, cursor_id,
                                        DataQueryAcceptType::SPARQL_JSON, operation, limit,
                                        &response));
    EXPECT_EQ(response, response_body);
}

/**
 * @brief Unit test for RDFoxAdapter to verify successful cursor deletion.
 */
TEST_F(RDFoxAdapterTest, DeleteCursorSuccess) {
    const std::string connection_id = RandomUtils::generateRandomString(8);
    const std::string cursor_id = RandomUtils::generateRandomString(8);
    expectRequests(*adapter_, {deleteCursorRequest(connection_id, cursor_id)});

    EXPECT_TRUE(adapter_->deleteCursor(connection_id, cursor_id));
}

/**
 * @brief Unit test for RDFoxAdapter to verify that loadRules loads the rules as datastore
 * content.
 */
TEST_F(RDFoxAdapterTest, LoadRulesSuccess) {
    expectRequests(*adapter_, {loadContentRequest("application/x.datalog", DATALOG_RULES)});

    EXPECT_TRUE(adapter_->loadRules(DATALOG_RULES, "application/x.datalog"));
}

/**
 * @brief Unit test for RDFoxAdapter to verify that loadRules propagates a failed load.
 */
TEST_F(RDFoxAdapterTest, LoadRulesFailure) {
    expectRequests(*adapter_, {loadContentRequest("application/x.datalog", DATALOG_RULES).fails()});

    EXPECT_FALSE(adapter_->loadRules(DATALOG_RULES, "application/x.datalog"));
}

// Unit tests for RDFoxAdapter to verify error handling of generic operations

/**
 * @brief Unit test for RDFoxAdapter to verify behavior when datastore creation
 * fails.
 */
TEST_F(RDFoxAdapterTest, FailedToCreateDataStore) {
    expectRequests(*adapter_,
                   {listDatastoresRequest("Name\r\n"), createDatastoreRequest().fails()});

    EXPECT_THROW(adapter_->initialize(), std::runtime_error);
}

/**
 * @brief Unit test for RDFoxAdapter to verify behavior when loading data fails.
 */
TEST_F(RDFoxAdapterTest, LoadDataFailure) {
    expectRequests(*adapter_, {loadDataRequest().fails()});

    EXPECT_FALSE(
        adapter_->loadData(TTL_DATA, reasonerSyntaxTypeToContentType(ReasonerSyntaxType::TURTLE)));
}

/**
 * @brief Unit test for RDFoxAdapter to verify behavior when a SPARQL query
 * fails.
 */
TEST_F(RDFoxAdapterTest, QueryDataFailure) {
    expectRequests(*adapter_, {queryRequest().respondsWith("").fails()});

    EXPECT_EQ(
        adapter_->queryData(SPARQL_QUERY, QueryLanguageType::SPARQL, DataQueryAcceptType::TEXT_TSV),
        "");
}

/**
 * @brief Unit test for RDFoxAdapter to verify the behavior when attempting to
 * delete a datastore fails.
 */
TEST_F(RDFoxAdapterTest, DeleteDataStoreFailure) {
    expectRequests(*adapter_,
                   {listDatastoresRequest("Name\r\nds-test\r\n"),
                    ExpectedRequest(http::verb::delete_, "/datastores/" + DATASTORE).fails()});

    EXPECT_FALSE(adapter_->deleteDataStore());
}

// Unit tests for RDFoxAdapter to verify cursor-related operations

/**
 * @brief Unit test for RDFoxAdapter to verify the behavior when a connection
 * cannot be created.
 */
TEST_F(RDFoxAdapterTest, CreateConnectionFailure) {
    expectRequests(*adapter_, {createConnectionRequest().respondsWithHeaders({}).fails()});

    EXPECT_THROW(adapter_->createConnection(), std::runtime_error);
}

/**
 * @brief Unit test for RDFoxAdapter to verify the behavior when header values
 * are missing after create connection.
 */
TEST_F(RDFoxAdapterTest, CreateConnectionMissingHeader) {
    const std::string header =
        RandomUtils::generateRandomInt(0, 1) == 0 ? "Location" : "RDFox-Authentication-Token";
    expectRequests(*adapter_, {createConnectionRequest().respondsWithHeaders(
                                  {{header, RandomUtils::generateRandomString(8)}})});

    EXPECT_THROW(adapter_->createConnection(), std::runtime_error);
}

/**
 * @brief Unit test for RDFoxAdapter to verify the behavior when the location
 * header format is incorrect after creating a connection.
 */
TEST_F(RDFoxAdapterTest, CreateConnectionWrongLocationHeaderFormat) {
    expectRequests(*adapter_,
                   {createConnectionRequest().respondsWithHeaders(
                       {{"Location", "wrong_format_without_slashes"},
                        {"RDFox-Authentication-Token", RandomUtils::generateRandomString(8)}})});

    EXPECT_THROW(adapter_->createConnection(), std::runtime_error);
}

/**
 * @brief Unit test for RDFoxAdapter to verify the behavior when a cursor cannot
 * be created.
 */
TEST_F(RDFoxAdapterTest, CreateCursorFailure) {
    const std::string connection_id = RandomUtils::generateRandomString(8);
    const std::string auth_token = RandomUtils::generateRandomString(8);
    const std::string invalid_query = RandomUtils::generateRandomString(20);
    expectRequests(*adapter_, {createCursorRequest(connection_id, auth_token, invalid_query)
                                   .respondsWithHeaders({})
                                   .fails()});

    EXPECT_THROW(adapter_->createCursor(connection_id, auth_token, invalid_query),
                 std::runtime_error);
}

/**
 * @brief Unit test for RDFoxAdapter to verify the behavior when location header
 * is missing after cursor creation.
 */
TEST_F(RDFoxAdapterTest, CreateCursorMissingLocationHeader) {
    const std::string connection_id = RandomUtils::generateRandomString(8);
    const std::string auth_token = RandomUtils::generateRandomString(8);
    const std::string valid_query = RandomUtils::generateRandomString(20);
    expectRequests(
        *adapter_,
        {createCursorRequest(connection_id, auth_token, valid_query).respondsWithHeaders({})});

    EXPECT_THROW(adapter_->createCursor(connection_id, auth_token, valid_query),
                 std::runtime_error);
}

/**
 * @brief Unit test for RDFoxAdapter to verify the behavior when the location
 * header format is incorrect after creating a cursor.
 */
TEST_F(RDFoxAdapterTest, CreateCursorWrongLocationHeaderFormat) {
    const std::string connection_id = RandomUtils::generateRandomString(8);
    const std::string auth_token = RandomUtils::generateRandomString(8);
    const std::string valid_query = RandomUtils::generateRandomString(20);
    expectRequests(*adapter_, {createCursorRequest(connection_id, auth_token, valid_query)
                                   .respondsWithHeaders({{"Location", "wrong_format"}})});

    EXPECT_THROW(adapter_->createCursor(connection_id, auth_token, valid_query),
                 std::runtime_error);
}

/**
 * @brief Unit test for RDFoxAdapter to verify the behavior when cursor advance
 * fails.
 */
TEST_F(RDFoxAdapterTest, AdvanceCursorFailure) {
    const std::string connection_id = RandomUtils::generateRandomString(8);
    const std::string auth_token = RandomUtils::generateRandomString(8);
    const std::string cursor_id = RandomUtils::generateRandomString(8);
    const std::string operation = randomOperation();
    const int limit = RandomUtils::generateRandomInt(1, 100);
    expectRequests(*adapter_,
                   {advanceCursorRequest(connection_id, auth_token, cursor_id, operation, limit)
                        .respondsWith("")
                        .fails()});

    std::string response;
    EXPECT_FALSE(adapter_->advanceCursor(connection_id, auth_token, cursor_id,
                                         DataQueryAcceptType::SPARQL_JSON, operation, limit,
                                         &response));
    EXPECT_EQ(response, "");
}

/**
 * @brief Unit test for RDFoxAdapter to verify the behavior when cursor advance
 * fails due to invalid operation value.
 */
TEST_F(RDFoxAdapterTest, AdvanceCursorInvalidOperation) {
    const std::string connection_id = RandomUtils::generateRandomString(8);
    const std::string auth_token = RandomUtils::generateRandomString(8);
    const std::string cursor_id = RandomUtils::generateRandomString(8);
    expectRequests(*adapter_, {});

    std::string response;
    EXPECT_FALSE(adapter_->advanceCursor(connection_id, auth_token, cursor_id,
                                         DataQueryAcceptType::SPARQL_JSON, "invalid_operation",
                                         std::nullopt, &response));
    EXPECT_EQ(response, "");
}

/**
 * @brief Unit test for RDFoxAdapter to verify the behavior when cursor deletion
 * fails.
 */
TEST_F(RDFoxAdapterTest, DeleteCursorFailure) {
    const std::string connection_id = RandomUtils::generateRandomString(8);
    const std::string cursor_id = RandomUtils::generateRandomString(8);
    expectRequests(*adapter_, {deleteCursorRequest(connection_id, cursor_id).fails()});

    EXPECT_FALSE(adapter_->deleteCursor(connection_id, cursor_id));
}
