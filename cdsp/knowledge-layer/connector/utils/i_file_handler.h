#ifndef I_FILE_HANDLER_H
#define I_FILE_HANDLER_H

#include <string>
#include <vector>

// Interface for file operations
class IFileHandler {
   public:
    virtual std::string readFile(const std::string& file_path) = 0;
    virtual std::vector<std::string> readDirectory(const std::string& directory_path) = 0;
    virtual void writeFile(const std::string& file_path, const std::string& content,
                           bool append_data = false) = 0;
    virtual ~IFileHandler() = default;
};

#endif  // I_FILE_HANDLER_H