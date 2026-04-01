#include "rdf4j_adapter.h"

#include <iostream>
#include <stdexcept>

RDF4JAdapter::RDF4JAdapter(const ReasonerServerData& server_data)
	: host_(server_data.host),
	  port_(server_data.port),
	  auth_header_base64_("Basic " + server_data.auth_base64) {
	if (server_data.data_store_name.has_value()) {
		data_store_ = server_data.data_store_name.value();
	} else {
		throw std::runtime_error("Data store name must be provided.");
	}
}

std::unique_ptr<RequestBuilder> RDF4JAdapter::createRequestBuilder() const {
	return std::make_unique<RequestBuilder>(host_, port_, auth_header_base64_);
}

void RDF4JAdapter::initialize() {
	std::cout << "** Initializing RDF4J reasoner adapter **" << std::endl;
	std::cout << " - Starting data store: " << data_store_ << std::endl;

	if (checkDataStore()) {
		std::cout << " - Data store '" + data_store_ + "' is already created." << std::endl;
		return;
	}

	std::cout << " - Data store '" << data_store_ << "' does not exist. Creating it..."
			  << std::endl;
	const std::string target = "/datastores/" + data_store_;

	if (createRequestBuilder()->setMethod(http::verb::post).setTarget(target).sendRequest()) {
		std::cout << " - Data store '" << data_store_ << "' created successfully." << std::endl;
		return;
	}

	throw std::runtime_error("Failed to create datastore '" + data_store_ + "'");
}

bool RDF4JAdapter::checkDataStore() {
	const std::string target = "/datastores";
	std::string response_body;

	if (!createRequestBuilder()
			 ->setMethod(http::verb::get)
			 .setTarget(target)
			 .setAcceptType("text/plain")
			 .sendRequest(nullptr, &response_body)) {
		return false;
	}

	return response_body.find(data_store_) != std::string::npos;
}

bool RDF4JAdapter::loadData(const std::string& data, const std::string& content_type) {
	const std::string target = "/datastores/" + data_store_ + "/content";

	return createRequestBuilder()
		->setMethod(http::verb::post)
		.setTarget(target)
		.setContentType(content_type)
		.setBody(data)
		.sendRequest();
}

std::string RDF4JAdapter::queryData(const std::string& query,
									 const QueryLanguageType& query_language_type,
									 const DataQueryAcceptType& accept_type) {
	const std::string target = "/datastores/" + data_store_ + "/sparql";
	std::string response_body;

	return createRequestBuilder()
				   ->setMethod(http::verb::post)
				   .setTarget(target)
				   .setContentType(queryLanguageTypeToContentType(query_language_type))
				   .setBody(query)
				   .setAcceptType(queryAcceptTypeToString(accept_type))
				   .sendRequest(nullptr, &response_body)
			   ? response_body
			   : "";
}

bool RDF4JAdapter::deleteDataStore() {
	if (!checkDataStore()) {
		std::cout << " - Data store '" + data_store_ + "' does not exists anymore." << std::endl;
		return true;
	}

	std::cout << " - Delete datastore endpoint is not exposed by the RDF4J reasoner service."
			  << std::endl;
	std::cout << " - Skipping datastore deletion for '" << data_store_ << "'." << std::endl;
	return true;
}
