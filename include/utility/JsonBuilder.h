#ifndef JSONBUILDER_H
#define JSONBUILDER_H

#include <sstream>
#include <string>

/**
 * @brief Class for building JSON formatted strings
 *
 * @description
 * JsonBuilder provides a streaming interface for creating JSON data structures.
 * It handles proper formatting of all standard JSON data types while maintaining
 * correct syntax. The builder keeps track of commas and formatting to ensure
 * valid JSON output.
 */
class JsonBuilder
{
public:
    JsonBuilder();

    /**
     * @brief Begins a new JSON object
     */
    void beginObject();

    /**
     * @brief Ends the current JSON object
     */
    void endObject();

    /**
     * @brief Begins a new JSON array
     */
    void beginArray();

    /**
     * @brief Ends the current JSON array
     */
    void endArray();

    /**
     * @brief Adds a key to the current object
     * @param key The key string
     */
    void addKey(const std::string& key);

    /**
     * @brief Adds a string value
     * @param value The string value
     */
    void addString(const std::string& value);

    /**
     * @brief Adds a number value
     * @param value The number value
     */
    void addNumber(float value);

    /**
     * @brief Adds a boolean value
     * @param value The boolean value
     */
    void addBool(bool value);

    /**
     * @brief Gets the built JSON string
     * @return The complete JSON string
     */
    std::string toString() const;

private:
    /**
     * @brief Escapes special characters in a string
     * @param str The string to escape
     * @return The escaped string
     */
    std::string escapeString(const std::string& str);

    std::stringstream m_stream;      ///< Stream for building the JSON string
    bool              m_needsComma;  ///< Flag indicating if a comma is needed before next value
};

#endif  // JSONBUILDER_H