#ifndef SERVER_DATA_FIXTURE_H
#define SERVER_DATA_FIXTURE_H

#include "data_types.h"
class ServerDataFixture {
   public:
    static WSServerData getValidWebsocketServerData();
    static ReasonerServerData getValidRDFoxServerData();
    static ReasonerServerData getValidRDF4JServerData();
    static ReasonerServerData getValidCQELSServerData();
};

#endif  // SERVER_DATA_FIXTURE_H