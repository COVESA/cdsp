#include "file_handler_impl.h"

#include <filesystem>
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
 * @brief Reads the contents of a directory and returns a list of file names.
 *
 * @param directory_path The path to the directory to be read.
 * @return A vector of strings containing the file names in the directory.
 * @throws std::runtime_error If the directory cannot be opened.
 */
std::vector<std::string> FileHandlerImpl::readDirectory(const std::string& directory_path) {
    std::vector<std::string> file_names;

    // Check if the given path is actually a directory
    if (!std::filesystem::is_directory(directory_path)) {
        throw std::runtime_error("Not a valid directory: " + directory_path);
    }

    // Iterate over directory entries and add file names to vector
    for (const auto& entry : std::filesystem::directory_iterator(directory_path)) {
        if (entry.is_regular_file()) {  // Ensure only files are added
            file_names.push_back(entry.path().filename().string());
        }
    }

    return file_names;
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

    // Ensure the parent directory exists
    std::filesystem::path path(file_path);
    std::filesystem::create_directories(path.parent_path());

    // Determine the open mode based on the 'append' boolean
    std::ios_base::openmode mode = append_data ? std::ios::app : std::ios::trunc;

    std::ofstream file(last_recorded_file_name_, mode);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + last_recorded_file_name_);
    }
    file << content;
}
