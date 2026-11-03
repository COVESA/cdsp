#ifndef I_REASONER_ADAPTER_H
#define I_REASONER_ADAPTER_H

#include <chrono>
#include <string>

#include "data_types.h"

class IReasonerAdapter {
   public:
    /**
     * @brief Connects to the reasoner and creates the datastore if it does not exist.
     */
    virtual void initialize() = 0;

    /**
     * @brief Loads static data (ontologies, SHACL shapes) into the datastore.
     * @return true if the data was loaded.
     */
    virtual bool loadData(const std::string& data, const std::string& content_type) = 0;

    /**
     * @brief Loads vehicle observations made at observation_time, as opposed to the static data of
     * loadData.
     *
     * Stream reasoners use observation_time for their windows; engines that keep all data in one
     * datastore may ignore it. The default loads the observations like loadData.
     * @return true if the observations were loaded.
     */
    virtual bool loadObservations(const std::string& data, const std::string& content_type,
                                  std::chrono::system_clock::time_point /*observation_time*/) {
        return loadData(data, content_type);
    }

    /**
     * @brief Loads reasoning rules (for example Datalog or SHACL rules).
     * @return true if the rules were loaded.
     */
    virtual bool loadRules(const std::string& rules, const std::string& content_type) = 0;

    /**
     * @brief Runs a query against the datastore.
     * @return The query result in the format given by accept_type, or an empty string on error.
     */
    virtual std::string queryData(const std::string& query,
                                  const QueryLanguageType& query_language_type,
                                  const DataQueryAcceptType& accept_type) = 0;

    /**
     * @brief Checks whether the datastore exists and is reachable.
     */
    virtual bool checkDataStore() = 0;

    /**
     * @brief Deletes the datastore.
     * @return true if the datastore was deleted or did not exist.
     */
    virtual bool deleteDataStore() = 0;
};

#endif  // I_REASONER_ADAPTER_H
