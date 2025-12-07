#include "systems/SSerialization.h"
#include <cctype>
#include <cmath>
#include "entities/Entity.h"

// ============================================================================
// SSerialization System Methods
// ============================================================================

void SSerialization::initialize()
{
    if (!m_initialized)
    {
        m_initialized = true;
    }
}

void SSerialization::shutdown()
{
    if (m_initialized)
    {
        m_initialized = false;
    }
}

SSerialization::JsonValue SSerialization::parse(const std::string& json)
{
    JsonParser parser(json);
    return JsonValue::parse(parser);
}

std::string SSerialization::build(const JsonValue& value)
{
    JsonBuilder builder;

    if (value.isNull())
    {
        // Null values are not directly supported by JsonBuilder
        return "null";
    }
    else if (value.isBool())
    {
        builder.addBool(value.getBool());
    }
    else if (value.isNumber())
    {
        builder.addNumber(static_cast<float>(value.getNumber()));
    }
    else if (value.isString())
    {
        builder.addString(value.getString());
    }
    else if (value.isArray())
    {
        builder.beginArray();
        const auto& arr = value.getArray();
        for (const auto& item : arr)
        {
            std::string itemJson = build(item);
            // For arrays/objects, we need to manually append the JSON string
            // This is a limitation of the current builder design
            // TODO: Improve builder to handle nested structures better
        }
        builder.endArray();
    }
    else if (value.isObject())
    {
        builder.beginObject();
        const auto& obj = value.getObject();
        for (const auto& [key, val] : obj)
        {
            builder.addKey(key);
            if (val.isBool())
                builder.addBool(val.getBool());
            else if (val.isNumber())
                builder.addNumber(static_cast<float>(val.getNumber()));
            else if (val.isString())
                builder.addString(val.getString());
            // For nested structures, recursion would be needed
        }
        builder.endObject();
    }

    return builder.toString();
}

SSerialization::JsonValue SSerialization::createObject()
{
    JsonValue val;
    val.m_value = JsonValue::Object{};
    return val;
}

SSerialization::JsonValue SSerialization::createArray()
{
    JsonValue val;
    val.m_value = JsonValue::Array{};
    return val;
}

void SSerialization::serializeEntity(const Entity& entity, JsonBuilder& builder)
{
    // This will be called by Entity::serialize
    // The actual serialization logic is in Entity.cpp
    // This method can be extended for additional orchestration if needed
}

void SSerialization::deserializeEntity(Entity& entity, const JsonValue& value)
{
    // This will be called by Entity::deserialize
    // The actual deserialization logic is in Entity.cpp
    // This method can be extended for additional orchestration if needed
}

// ============================================================================
// JsonParser Implementation
// ============================================================================

SSerialization::JsonParser SSerialization::JsonParser::fromFile(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open file: " + path);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return JsonParser(buffer.str());
}

bool SSerialization::JsonParser::hasNext() const
{
    return m_pos < m_json.length();
}

void SSerialization::JsonParser::beginObject()
{
    skipWhitespace();
    if (!hasNext() || m_json[m_pos++] != '{')
        throw std::runtime_error("Expected '{'");
}

void SSerialization::JsonParser::endObject()
{
    skipWhitespace();
    if (!hasNext() || m_json[m_pos++] != '}')
        throw std::runtime_error("Expected '}'");
}

void SSerialization::JsonParser::beginArray()
{
    skipWhitespace();
    if (!hasNext() || m_json[m_pos++] != '[')
        throw std::runtime_error("Expected '['");
}

void SSerialization::JsonParser::endArray()
{
    skipWhitespace();
    if (!hasNext() || m_json[m_pos++] != ']')
        throw std::runtime_error("Expected ']'");
}

std::string SSerialization::JsonParser::getKey()
{
    skipWhitespace();
    if (m_json[m_pos] == ',')
    {
        m_pos++;
        skipWhitespace();
    }
    std::string key = getString();
    skipWhitespace();
    if (!hasNext() || m_json[m_pos++] != ':')
        throw std::runtime_error("Expected ':'");
    return key;
}

std::string SSerialization::JsonParser::getString()
{
    skipWhitespace();
    if (!hasNext() || m_json[m_pos++] != '"')
        throw std::runtime_error("Expected '\"'");

    std::string value;
    value.reserve(32);  // Reserve some space to avoid small reallocations
    bool escaped = false;

    while (hasNext())
    {
        char c = m_json[m_pos++];
        if (escaped)
        {
            switch (c)
            {
                case '"':
                    value += '"';
                    break;
                case '\\':
                    value += '\\';
                    break;
                case '/':
                    value += '/';
                    break;
                case 'b':
                    value += '\b';
                    break;
                case 'f':
                    value += '\f';
                    break;
                case 'n':
                    value += '\n';
                    break;
                case 'r':
                    value += '\r';
                    break;
                case 't':
                    value += '\t';
                    break;
                default:
                    throw std::runtime_error("Invalid escape sequence");
            }
            escaped = false;
        }
        else if (c == '\\')
        {
            escaped = true;
        }
        else if (c == '"')
        {
            return value;
        }
        else
        {
            value += c;
        }
    }
    throw std::runtime_error("Unterminated string");
}

float SSerialization::JsonParser::getNumber()
{
    skipWhitespace();
    size_t start         = m_pos;
    bool   hasDecimal    = false;
    int    decimalPlaces = 0;

    // Handle negative numbers
    if (m_json[m_pos] == '-')
        m_pos++;

    // Parse integer part
    while (m_pos < m_json.length() && std::isdigit(m_json[m_pos]))
        m_pos++;

    // Parse decimal part
    if (m_pos < m_json.length() && m_json[m_pos] == '.')
    {
        hasDecimal = true;
        m_pos++;  // skip decimal point
        while (m_pos < m_json.length() && std::isdigit(m_json[m_pos]))
        {
            decimalPlaces++;
            m_pos++;
        }
    }

    // Parse exponent
    if (m_pos < m_json.length() && (m_json[m_pos] == 'e' || m_json[m_pos] == 'E'))
    {
        m_pos++;
        if (m_pos < m_json.length() && (m_json[m_pos] == '+' || m_json[m_pos] == '-'))
            m_pos++;
        while (m_pos < m_json.length() && std::isdigit(m_json[m_pos]))
            m_pos++;
    }

    std::string numStr = m_json.substr(start, m_pos - start);
    float       value  = std::stof(numStr);

    // Round to the original number of decimal places
    if (hasDecimal)
    {
        float multiplier = std::pow(10.0f, decimalPlaces);
        value            = std::round(value * multiplier) / multiplier;
    }

    return value;
}

bool SSerialization::JsonParser::getBool()
{
    skipWhitespace();
    if (m_pos + 4 <= m_json.length() && m_json.substr(m_pos, 4) == "true")
    {
        m_pos += 4;
        return true;
    }
    else if (m_pos + 5 <= m_json.length() && m_json.substr(m_pos, 5) == "false")
    {
        m_pos += 5;
        return false;
    }
    throw std::runtime_error("Expected 'true' or 'false'");
}

void SSerialization::JsonParser::getNull()
{
    skipWhitespace();
    if (m_pos + 4 > m_json.length() || m_json.substr(m_pos, 4) != "null")
    {
        throw std::runtime_error("Expected 'null'");
    }
    m_pos += 4;
}

char SSerialization::JsonParser::peek()
{
    skipWhitespace();
    return m_pos < m_json.length() ? m_json[m_pos] : '\0';
}

void SSerialization::JsonParser::consume()
{
    if (m_pos < m_json.length())
    {
        m_pos++;
    }
}

void SSerialization::JsonParser::skipWhitespace()
{
    while (m_pos < m_json.length() && std::isspace(m_json[m_pos]))
    {
        m_pos++;
    }
}

// ============================================================================
// JsonBuilder Implementation
// ============================================================================

SSerialization::JsonBuilder::JsonBuilder() : m_needsComma(false) {}

void SSerialization::JsonBuilder::beginObject()
{
    if (m_needsComma)
        m_stream << ",";
    m_stream << "{";
    m_needsComma = false;
}

void SSerialization::JsonBuilder::endObject()
{
    m_stream << "}";
    m_needsComma = true;
}

void SSerialization::JsonBuilder::beginArray()
{
    if (m_needsComma)
        m_stream << ",";
    m_stream << "[";
    m_needsComma = false;
}

void SSerialization::JsonBuilder::endArray()
{
    m_stream << "]";
    m_needsComma = true;
}

void SSerialization::JsonBuilder::addKey(const std::string& key)
{
    if (m_needsComma)
        m_stream << ",";
    m_stream << "\"" << escapeString(key) << "\":";
    m_needsComma = false;
}

void SSerialization::JsonBuilder::addString(const std::string& value)
{
    if (m_needsComma)
        m_stream << ",";
    m_stream << "\"" << escapeString(value) << "\"";
    m_needsComma = true;
}

void SSerialization::JsonBuilder::addNumber(float value)
{
    if (m_needsComma)
        m_stream << ",";
    m_stream << value;
    m_needsComma = true;
}

void SSerialization::JsonBuilder::addBool(bool value)
{
    if (m_needsComma)
        m_stream << ",";
    m_stream << (value ? "true" : "false");
    m_needsComma = true;
}

std::string SSerialization::JsonBuilder::toString() const
{
    return m_stream.str();
}

std::string SSerialization::JsonBuilder::escapeString(const std::string& str)
{
    std::string result;
    for (char c : str)
    {
        switch (c)
        {
            case '\"':
                result += "\\\"";
                break;
            case '\\':
                result += "\\\\";
                break;
            case '\b':
                result += "\\b";
                break;
            case '\f':
                result += "\\f";
                break;
            case '\n':
                result += "\\n";
                break;
            case '\r':
                result += "\\r";
                break;
            case '\t':
                result += "\\t";
                break;
            default:
                result += c;
        }
    }
    return result;
}

// ============================================================================
// JsonValue Implementation
// ============================================================================

const SSerialization::JsonValue SSerialization::JsonValue::NULL_VALUE;

SSerialization::JsonValue::JsonValue(const std::string& json)
{
    JsonParser parser(json);
    *this = parse(parser);
}

bool SSerialization::JsonValue::getBool(bool defaultValue) const
{
    return isBool() ? std::get<bool>(m_value) : defaultValue;
}

double SSerialization::JsonValue::getNumber(double defaultValue) const
{
    return isNumber() ? std::get<double>(m_value) : defaultValue;
}

std::string SSerialization::JsonValue::getString(const std::string& defaultValue) const
{
    return isString() ? std::get<std::string>(m_value) : defaultValue;
}

const SSerialization::JsonValue::Array& SSerialization::JsonValue::getArray() const
{
    static const Array empty;
    return isArray() ? std::get<Array>(m_value) : empty;
}

const SSerialization::JsonValue::Object& SSerialization::JsonValue::getObject() const
{
    static const Object empty;
    return isObject() ? std::get<Object>(m_value) : empty;
}

const SSerialization::JsonValue& SSerialization::JsonValue::operator[](const std::string& key) const
{
    if (!isObject())
        return NULL_VALUE;
    const auto& obj = getObject();
    auto        it  = obj.find(key);
    return it != obj.end() ? it->second : NULL_VALUE;
}

const SSerialization::JsonValue& SSerialization::JsonValue::operator[](size_t index) const
{
    if (!isArray())
        return NULL_VALUE;
    const auto& arr = getArray();
    return index < arr.size() ? arr[index] : NULL_VALUE;
}

SSerialization::JsonValue SSerialization::JsonValue::parse(JsonParser& parser)
{
    JsonValue value;
    parser.skipWhitespace();

    if (!parser.hasNext())
        return value;

    char c = parser.peek();

    switch (c)
    {
        case '{':
            value.m_value = parseObject(parser);
            break;
        case '[':
        {
            Array arr     = parseArray(parser);
            value.m_value = std::move(arr);
        }
        break;
        case '"':
            value.m_value = parser.getString();
            break;
        case 't':
        case 'f':
            try
            {
                value.m_value = parser.getBool();
            }
            catch (const std::runtime_error&)
            {
                throw std::runtime_error("Expected 'true', 'false', or a string in quotes");
            }
            break;
        case 'n':
            try
            {
                parser.getNull();
                value.m_value = nullptr;
            }
            catch (const std::runtime_error&)
            {
                throw std::runtime_error("Expected 'null' or a string in quotes");
            }
            break;
        default:
            if (c == '-' || (c >= '0' && c <= '9'))
            {
                value.m_value = static_cast<double>(parser.getNumber());
            }
            else
            {
                throw std::runtime_error("Unexpected character in JSON");
            }
            break;
    }
    return value;
}

SSerialization::JsonValue::Array SSerialization::JsonValue::parseArray(JsonParser& parser)
{
    Array array;
    parser.beginArray();
    parser.skipWhitespace();

    while (parser.hasNext())
    {
        if (parser.peek() == ']')
        {
            parser.endArray();
            return array;
        }

        array.push_back(parse(parser));
        parser.skipWhitespace();

        char next = parser.peek();
        if (next == ']')
        {
            parser.endArray();
            return array;
        }
        else if (next == ',')
        {
            parser.consume();
            parser.skipWhitespace();
        }
        else
        {
            throw std::runtime_error("Expected ',' or ']' after array element");
        }
    }

    throw std::runtime_error("Unterminated array");
}

SSerialization::JsonValue::Object SSerialization::JsonValue::parseObject(JsonParser& parser)
{
    Object object;
    parser.beginObject();

    while (parser.hasNext())
    {
        parser.skipWhitespace();
        if (parser.peek() == '}')
        {
            parser.endObject();
            return object;
        }

        std::string key = parser.getKey();
        parser.skipWhitespace();
        JsonValue value = parse(parser);
        object.emplace(key, std::move(value));

        parser.skipWhitespace();
        if (parser.peek() == '}')
        {
            parser.endObject();
            return object;
        }
        else if (parser.peek() == ',')
        {
            parser.consume();
            parser.skipWhitespace();
            continue;
        }
        throw std::runtime_error("Expected ',' or '}'");
    }

    throw std::runtime_error("Unterminated object");
}

bool SSerialization::JsonValue::hasKey(const std::string& key) const
{
    if (!isObject())
        return false;
    const auto& obj = getObject();
    return obj.find(key) != obj.end();
}
