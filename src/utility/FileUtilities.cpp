#include "FileUtilities.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

std::string FileUtilities::readFile(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        throw std::runtime_error("Could not open file: " + path);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void FileUtilities::writeFile(const std::string& path, const std::string& content)
{
    std::ofstream file(path);
    if (!file.is_open())
    {
        throw std::runtime_error("Could not open file for writing: " + path);
    }
    file << content;
}
