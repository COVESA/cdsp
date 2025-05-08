#ifndef SERVER_DATA_FIXTURE_H
#define SERVER_DATA_FIXTURE_H

#include "data_types.h"
class ServerDataFixture {
   public:
    static WSServerData getValidWebsocketServerData();
    static ReasonerServerData getValidRDFoxServerData();
};

#endif  // SERVER_DATA_FIXTURE_H