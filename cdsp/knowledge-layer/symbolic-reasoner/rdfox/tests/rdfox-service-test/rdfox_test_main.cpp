#include <boost/asio.hpp>
#include <boost/asio/connect.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <iostream>
#include <string>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

const std::string RDFOX_SERVER = "localhost";
const std::string PORT = "12110";
const std::string AUTH_HEADER =
    "Basic cm9vdDphZG1pbg==";  // Example for 'root:admin' encoded in base64

// Helper function to handle response status codes
void assertResponseOk(const http::response<http::string_body>& res, const std::string& message) {
    if (res.result() != http::status::ok && res.result() != http::status::created &&
        res.result() != http::status::no_content) {
        std::cerr << message << "\nStatus: " << res.result_int() << "\n" << res.body() << std::endl;
        throw std::runtime_error("Request failed.");
    }
}

// Function to perform a POST request
http::response<http::string_body> postRequest(tcp::socket& socket, beast::flat_buffer& buffer,
                                              const std::string& target, const std::string& body,
                                              const std::string& content_type) {
    try {
        http::request<http::string_body> req{http::verb::post, target, 11};
        req.set(http::field::host, RDFOX_SERVER);
        req.set(http::field::authorization, AUTH_HEADER);
        req.set(http::field::content_type, content_type);
        req.set(http::field::connection, "keep-alive");  // Keep connection alive
        req.body() = body;
        req.prepare_payload();

        http::write(socket, req);
        http::response<http::string_body> res;
        http::read(socket, buffer, res);

        return res;
    } catch (const std::exception& e) {
        std::cerr << "Error in POST request: " << e.what() << std::endl;
        throw;
    }
}

// Function to perform a GET request
http::response<http::string_body> getRequest(tcp::socket& socket, beast::flat_buffer& buffer,
                                             const std::string& target,
                                             const std::string& accept_type) {
    try {
        http::request<http::empty_body> req{http::verb::get, target, 11};
        req.set(http::field::host, RDFOX_SERVER);
        req.set(http::field::authorization, AUTH_HEADER);
        req.set(http::field::accept, accept_type);
        req.set(http::field::connection, "keep-alive");  // Keep connection alive

        http::write(socket, req);
        http::response<http::string_body> res;
        http::read(socket, buffer, res);

        return res;
    } catch (const std::exception& e) {
        std::cerr << "Error in GET request: " << e.what() << std::endl;
        throw;
    }
}

// Function to perform a DELETE request
http::response<http::string_body> deleteRequest(tcp::socket& socket, beast::flat_buffer& buffer,
                                                const std::string& target) {
    try {
        http::request<http::empty_body> req{http::verb::delete_, target, 11};
        req.set(http::field::host, RDFOX_SERVER);
        req.set(http::field::authorization, AUTH_HEADER);
        req.set(http::field::connection, "keep-alive");  // Keep connection alive

        http::write(socket, req);
        http::response<http::string_body> res;
        http::read(socket, buffer, res);

        return res;
    } catch (const std::exception& e) {
        std::cerr << "Error in DELETE request: " << e.what() << std::endl;
        throw;
    }
}

// Main function to use persistent connections
int main() {
    try {
        net::io_context ioc;
        tcp::resolver resolver(ioc);
        tcp::socket socket(ioc);
        auto const results = resolver.resolve(RDFOX_SERVER, PORT);
        net::connect(socket, results.begin(), results.end());
        beast::flat_buffer buffer;

        std::string store_name = "family";

        // Step 1: Check if the 'family' data store exists and delete it if necessary
        auto res = getRequest(socket, buffer, "/datastores", "application/json");
        assertResponseOk(res, "Failed to obtain list of datastores.");
        std::cout << "Data store list:\n" << res.body() << std::endl;

        if (res.body().find(store_name) != std::string::npos) {
            std::cout << "Data store '" << store_name << "' exists. Deleting..." << std::endl;
            res = deleteRequest(socket, buffer, "/datastores/" + store_name);
            assertResponseOk(res, "Failed to delete data store.");
            std::cout << "Data store '" << store_name << "' deleted successfully." << std::endl;
        } else {
            std::cout << "Data store '" << store_name << "' does not exist." << std::endl;
        }

        // Step 2: Create a new data store
        std::cout << "Creating a new data store '" << store_name << "'." << std::endl;
        res = postRequest(socket, buffer, "/datastores/family?type=parallel-ww", "",
                          "application/json");
        assertResponseOk(res, "Failed to create datastore.");
        std::cout << "Data store created." << std::endl;

        // Step 3: Add facts in Turtle format
        std::string turtle_data = R"(
            @prefix : <https://oxfordsemantic.tech/RDFox/getting-started/> .
            :peter :forename "Peter" ;
                a :Person ;
                :marriedTo :lois ;
                :gender "male" .
            :lois :forename "Lois" ;
                a :Person ;
                :gender "female" .
            :meg :forename "Meg" ;
                a :Person ;
                :hasParent :lois, :peter ;
                :gender "female" .
            :chris :forename "Chris" ;
                a :Person ;
                :hasParent :peter ;
                :gender "male" .
            :stewie :forename "Stewie" ;
                a :Person ;
                :hasParent :lois ;
                :gender "male" .
            :brian :forename "Brian" .
        )";
        res = postRequest(socket, buffer, "/datastores/family/content", turtle_data, "text/turtle");
        assertResponseOk(res, "Failed to add facts to data store.");
        std::cout << "Facts added." << std::endl;

        // Step 4: Run a SPARQL query
        std::string sparql_query = R"(
            PREFIX : <https://oxfordsemantic.tech/RDFox/getting-started/> 
            SELECT ?p ?n WHERE { ?p a :Person . ?p :forename ?n }
        )";
        res = postRequest(socket, buffer, "/datastores/family/sparql", sparql_query,
                          "application/sparql-query");
        assertResponseOk(res, "Failed to run SPARQL query.");
        std::cout << "SPARQL query result:\n" << res.body() << std::endl;

        res = deleteRequest(socket, buffer, "/datastores/" + store_name);
        assertResponseOk(res, "Failed to delete data store.");
        std::cout << "Data store '" << store_name << "' deleted successfully." << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return 0;
}