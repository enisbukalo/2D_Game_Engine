#ifndef SSERIALIZATION_H
#define SSERIALIZATION_H

#include <fstream>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

#include <Entity.h>
#include "JsonBuilder.h"
#include "JsonParser.h"

namespace Serialization
{

/**
 * @brief Serialization system for JSON parsing, building, and entity/component serialization
 *
 * @description
 * SSerialization is a singleton system that centralizes all JSON and serialization
 * operations in the game engine. It provides:
 * - JSON parsing (string to structured data)
 * - JSON building (structured data to string)
 * - Entity and component serialization/deserialization orchestration
 *
 * Usage:
 *   auto& serializer = Serialization::SSerialization::instance();
 *   JsonValue value = serializer.parse("{\"key\": \"value\"}");
 *   std::string json = serializer.build(value);
 */
class SSerialization
{
public:
    class JsonValue;

    /**
     * @brief Gets the singleton instance of SSerialization
     * @return Reference to the SSerialization singleton
     */
    static SSerialization& instance()
    {
        static SSerialization s;
        return s;
    }

    /**
     * @brief Initializes the serialization system
     * Called during engine startup if needed
     */
    void initialize();

    /**
     * @brief Shuts down the serialization system
     * Called during engine shutdown if needed
     */
    void shutdown();

    /**
     * @brief Parses a JSON string into a JsonValue
     * @param json The JSON string to parse
     * @return JsonValue representing the parsed data
     * @throws std::runtime_error if parsing fails
     */
    JsonValue parse(const std::string& json);

    /**
     * @brief Builds a JSON string from a JsonValue
     * @param value The JsonValue to serialize
     * @return JSON string representation
     */
    std::string build(const JsonValue& value);

    /**
     * @brief Creates an empty JSON object
     * @return JsonValue representing an empty object
     */
    JsonValue createObject();

    /**
     * @brief Creates an empty JSON array
     * @return JsonValue representing an empty array
     */
    JsonValue createArray();
    /**
     * @brief Serializes an entity and its components to a JsonBuilder
     * @param entity The entity ID to serialize
     * @param builder The builder to write to
     * @param registry Registry to access entity components
     */
    void serializeEntity(Entity entity, JsonBuilder& builder, class Registry& registry);

    /**
     * @brief Deserializes an entity from a JsonValue
     * @param entity The entity ID to deserialize into
     * @param value The JSON data to read from
     * @param registry Registry to add components to
     */
    void deserializeEntity(Entity entity, const JsonValue& value, class Registry& registry);

    // Deleted copy/move constructors and assignment operators
    SSerialization(const SSerialization&)            = delete;
    SSerialization& operator=(const SSerialization&) = delete;
    SSerialization(SSerialization&&)                 = delete;
    SSerialization& operator=(SSerialization&&)      = delete;

    /**
     * @brief JsonValue - represents a JSON value (null, bool, number, string, array, or object)
     */
    class JsonValue
    {
    public:
        using Object = std::map<std::string, JsonValue>;
        using Array  = std::vector<JsonValue>;
        using Value  = std::variant<std::nullptr_t, bool, double, std::string, Array, Object>;

        JsonValue() : m_value(nullptr) {}
        JsonValue(const JsonValue& other) : m_value(other.m_value) {}
        JsonValue(JsonValue&& other) noexcept : m_value(std::move(other.m_value)) {}

        JsonValue& operator=(const JsonValue& other)
        {
            m_value = other.m_value;
            return *this;
        }

        JsonValue& operator=(JsonValue&& other) noexcept
        {
            m_value = std::move(other.m_value);
            return *this;
        }

        explicit JsonValue(const std::string& json);

        // Type checking
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

        // Value getters
        bool          getBool(bool defaultValue = false) const;
        double        getNumber(double defaultValue = 0.0) const;
        std::string   getString(const std::string& defaultValue = "") const;
        const Array&  getArray() const;
        const Object& getObject() const;

        // Convenience accessors
        const JsonValue& operator[](const std::string& key) const;
        const JsonValue& operator[](size_t index) const;
        bool             hasKey(const std::string& key) const;

        // Static parsing helpers
        static JsonValue parse(JsonParser& parser);
        static JsonValue parseValue(JsonParser& parser);
        static Array     parseArray(JsonParser& parser);
        static Object    parseObject(JsonParser& parser);

    private:
        friend class SSerialization;
        Value                  m_value;
        static const JsonValue NULL_VALUE;
    };

private:
    SSerialization()  = default;
    ~SSerialization() = default;

    bool m_initialized = false;
};

}  // namespace Serialization

#endif  // SSERIALIZATION_H
