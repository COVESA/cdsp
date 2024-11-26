#ifndef INIT_CONFIG_FIXTURE_H
#define INIT_CONFIG_FIXTURE_H

#include "data_types.h"
class InitConfigFixture {
   public:
    static InitConfig getValidInitConfig(const std::string& oid);

   private:
    static std::string generateUUID();
};

#endif  // INIT_CONFIG_FIXTURE_H