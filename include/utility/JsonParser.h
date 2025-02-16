#ifndef JSONPARSER_H
#define JSONPARSER_H

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

/**
 * @brief Class for parsing JSON formatted strings
 *
 * @description
 * JsonParser provides a streaming interface for reading JSON data structures.
 * It handles parsing of all standard JSON data types while maintaining strict
 * syntax checking. The parser keeps track of its position in the input string
 * and provides methods to extract values while automatically handling whitespace
 * and structural elements. Error handling is done through exceptions to ensure
 * data integrity.
 */
class JsonParser
{
public:
    /**
     * @brief Constructs a JsonParser with a JSON string
     * @param json The JSON string to parse
     */
    explicit JsonParser(const std::string& json) : m_json(json), m_pos(0) {}

    /**
     * @brief Creates a JsonParser by reading from a file
     * @param path The path to the JSON file to parse
     * @return A JsonParser instance initialized with the file contents
     * @throws std::runtime_error if the file cannot be opened or read
     */
    static JsonParser fromFile(const std::string& path);

    /**
     * @brief Checks if there is more content to parse
     * @return true if there is more content, false if at end
     */
    bool hasNext() const;

    /**
     * @brief Expects and consumes a '{' character
     * @throw std::runtime_error if '{' is not found
     */
    void beginObject();

    /**
     * @brief Expects and consumes a '}' character
     * @throw std::runtime_error if '}' is not found
     */
    void endObject();

    /**
     * @brief Expects and consumes a '[' character
     * @throw std::runtime_error if '[' is not found
     */
    void beginArray();

    /**
     * @brief Expects and consumes a ']' character
     * @throw std::runtime_error if ']' is not found
     */
    void endArray();

    /**
     * @brief Gets the next key in a JSON object
     * @return The key string
     * @throw std::runtime_error if key format is invalid
     */
    std::string getKey();

    /**
     * @brief Gets the next string value
     * @return The parsed string value
     * @throw std::runtime_error if string format is invalid
     */
    std::string getString();

    /**
     * @brief Gets the next number value
     * @return The parsed floating point value with original decimal precision
     * @throw std::runtime_error if number format is invalid
     */
    float getNumber();

    /**
     * @brief Gets the next boolean value
     * @return The parsed boolean value
     * @throw std::runtime_error if boolean format is invalid
     */
    bool getBool();

    // New methods
    void getNull();
    char peek();
    void consume();
    void skipWhitespace();

private:
    std::string m_json;  ///< The JSON string being parsed
    size_t      m_pos;   ///< Current position in the JSON string
};

#endif  // JSONPARSER_H