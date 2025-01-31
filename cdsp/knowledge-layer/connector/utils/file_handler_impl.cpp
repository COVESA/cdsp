#include "file_handler_impl.h"

#include <fstream>
#include <iostream>
#include <stdexcept>

/**
 * @brief Reads the contents of a file and returns it as a string.
 *
 * @param file_path The path to the file to be read.
 * @return A string containing the contents of the file.
 * @throws std::runtime_error If the file cannot be opened.
 */
std::string FileHandlerImpl::readFile(const std::string& file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to read file: " + file_path);
    }
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

/**
 * @brief Writes the given content to a file specified by the file path, with support for appending
 * or overwriting data.
 *
 * @param file_path The path to the file where the content will be written.
 * @param content The content to be written to the file.
 * @param append_data A boolean indicating whether to append the content (`true`)
 *                    or overwrite the file (`false`). The mode is reset to overwrite if
 *                    the file path differs from the last recorded file name.
 * @throws std::runtime_error If the file cannot be opened for writing.
 */
void FileHandlerImpl::writeFile(const std::string& file_path, const std::string& content,
                                bool append_data) {
    if (file_path != last_recorded_file_name_) {
        last_recorded_file_name_ = file_path;
        append_data = false;
    }

    // Determine the open mode based on the 'append' boolean
    std::ios_base::openmode mode = append_data ? std::ios::app : std::ios::trunc;

    std::ofstream file(last_recorded_file_name_, mode);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + last_recorded_file_name_);
    }
    file << content;
}
