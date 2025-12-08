#ifndef FILE_UTILITIES_H
#define FILE_UTILITIES_H

#include <string>

namespace Internal
{

/**
 * @brief Utility class for file operations
 *
 * @description
 * FileUtilities provides static methods for common file operations
 * such as reading from and writing to files. All methods handle
 * file access errors through exceptions.
 **/
class FileUtilities
{
public:
    /**
     * @brief Reads the entire contents of a file into a string
     * @param path The path to the file to read
     * @return The contents of the file as a string
     * @throws std::runtime_error if the file cannot be opened or read
     */
    static std::string readFile(const std::string& path);

    /**
     * @brief Writes a string to a file, overwriting any existing content
     * @param path The path to the file to write
     * @param content The content to write to the file
     * @throws std::runtime_error if the file cannot be opened or written to
     */
    static void writeFile(const std::string& path, const std::string& content);
};

}  // namespace Internal

#endif
