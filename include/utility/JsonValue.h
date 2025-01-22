#ifndef JSONVALUE_H
#define JSONVALUE_H

#include <map>
#include <memory>
#include <string>
#include <variant>
#include <vector>
#include "JsonParser.h"

/**
 * @brief Class for representing and manipulating JSON values
 *
 * @description
 * JsonValue provides a type-safe container for JSON data structures.
 * It can hold any valid JSON value type (null, boolean, number, string, array, or object)
 * and provides methods for type checking and safe value access.
 * The class uses std::variant internally to store the different possible value types
 * and provides a convenient interface for accessing and manipulating JSON data.
 */
class JsonValue
{
public:
    /**
     * @brief Type definitions for JSON data structures
     */
    using Object = std::map<std::string, JsonValue>;
    using Array  = std::vector<JsonValue>;
    using Value  = std::variant<std::nullptr_t, bool, double, std::string, Array, Object>;

    /**
     * @brief Default constructor creates a null value
     */
    JsonValue() : m_value(nullptr) {}

    /**
     * @brief Copy constructor
     * @param other The JsonValue to copy from
     */
    JsonValue(const JsonValue& other) : m_value(other.m_value) {}

    /**
     * @brief Move constructor
     * @param other The JsonValue to move from
     */
    JsonValue(JsonValue&& other) noexcept : m_value(std::move(other.m_value)) {}

    /**
     * @brief Copy assignment operator
     * @param other The JsonValue to copy from
     * @return Reference to this object
     */
    JsonValue& operator=(const JsonValue& other)
    {
        m_value = other.m_value;
        return *this;
    }

    /**
     * @brief Move assignment operator
     * @param other The JsonValue to move from
     * @return Reference to this object
     */
    JsonValue& operator=(JsonValue&& other) noexcept
    {
        m_value = std::move(other.m_value);
        return *this;
    }

    /**
     * @brief Constructs a JsonValue by parsing a JSON string
     * @param json The JSON string to parse
     */
    JsonValue(const std::string& json);

    /**
     * @brief Type checking methods
     * @return true if the value is of the specified type, false otherwise
     */
    bool isNull() const
    {
        return std::holds_alternative<std::nullptr_t>(m_value);
    }
    bool isBool() const
    {
        return std::holds_alternative<bool>(m_value);
    }
    bool isNumber() const
    {
        return std::holds_alternative<double>(m_value);
    }
    bool isString() const
    {
        return std::holds_alternative<std::string>(m_value);
    }
    bool isArray() const
    {
        return std::holds_alternative<Array>(m_value);
    }
    bool isObject() const
    {
        return std::holds_alternative<Object>(m_value);
    }

    /**
     * @brief Value getters with default values
     * @param defaultValue The value to return if the stored type doesn't match
     * @return The stored value if types match, otherwise the default value
     */
    bool          getBool(bool defaultValue = false) const;
    double        getNumber(double defaultValue = 0.0) const;
    std::string   getString(const std::string& defaultValue = "") const;
    const Array&  getArray() const;
    const Object& getObject() const;

    /**
     * @brief Convenience accessors for objects and arrays
     * @param key The key to look up in an object
     * @param index The index to look up in an array
     * @return Reference to the value if found, or to a null value if not found
     */
    const JsonValue& operator[](const std::string& key) const;
    const JsonValue& operator[](size_t index) const;

    /**
     * @brief Static helper methods for parsing JSON structures
     */
    static JsonValue parse(JsonParser& parser);
    static JsonValue parseValue(JsonParser& parser);
    static Array     parseArray(JsonParser& parser);
    static Object    parseObject(JsonParser& parser);

private:
    Value                  m_value;     ///< The stored JSON value
    static const JsonValue NULL_VALUE;  ///< Static null value for returning when key/index not found
};

#endif  // JSONVALUE_H