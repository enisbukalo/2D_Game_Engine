#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <cmath>
#include <fstream>
#include <sstream>
#include <string>

// Helper function to read file content
inline std::string readFile(const std::string& path)
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

// Helper function to write file content
inline void writeFile(const std::string& path, const std::string& content)
{
    std::ofstream file(path);
    if (!file.is_open())
    {
        throw std::runtime_error("Could not open file for writing: " + path);
    }
    file << content;
}

// Helper function for floating point comparison
inline bool approxEqual(double a, double b, double epsilon = 0.0001)
{
    return std::fabs(a - b) < epsilon;
}

#endif  // TEST_UTILS_H