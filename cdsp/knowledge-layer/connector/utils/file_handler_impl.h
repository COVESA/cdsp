#ifndef FILE_HANDLER_IMPL_H
#define FILE_HANDLER_IMPL_H

#include <string>

#include "i_file_handler.h"

class FileHandlerImpl : public IFileHandler {
   public:
    std::string readFile(const std::string& file_path) override;
    std::vector<std::string> readDirectory(const std::string& directory_path) override;
    void writeFile(const std::string& file_path, const std::string& content,
                   bool append_data) override;

   private:
    std::string last_recorded_file_name_;
};

#endif  // FILE_HANDLER_IMPL_H