#ifndef MOCK_I_FILE_HANDLER_H
#define MOCK_I_FILE_HANDLER_H

#include <gmock/gmock.h>

#include "i_file_handler.h"

class MockIFileHandler : public IFileHandler {
   public:
    MOCK_METHOD(std::string, readFile, (const std::string& file_path), (override));
    MOCK_METHOD(std::vector<std::string>, readDirectory, (const std::string& directory_path),
                (override));
    MOCK_METHOD(void, writeFile,
                (const std::string& file_path, const std::string& content, bool append_data),
                (override));
};
#endif  // MOCK_I_FILE_HANDLER_H
