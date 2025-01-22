#include "JsonValue.h"
#include "JsonParser.h"

// Static null value for returning when key/index not found
const JsonValue JsonValue::NULL_VALUE;

JsonValue::JsonValue(const std::string& json)
{
    JsonParser parser(json);
    *this = parse(parser);
}

bool JsonValue::getBool(bool defaultValue) const
{
    return isBool() ? std::get<bool>(m_value) : defaultValue;
}

double JsonValue::getNumber(double defaultValue) const
{
    return isNumber() ? std::get<double>(m_value) : defaultValue;
}

std::string JsonValue::getString(const std::string& defaultValue) const
{
    return isString() ? std::get<std::string>(m_value) : defaultValue;
}

const JsonValue::Array& JsonValue::getArray() const
{
    static const Array empty;
    return isArray() ? std::get<Array>(m_value) : empty;
}

const JsonValue::Object& JsonValue::getObject() const
{
    static const Object empty;
    return isObject() ? std::get<Object>(m_value) : empty;
}

const JsonValue& JsonValue::operator[](const std::string& key) const
{
    if (!isObject())
        return NULL_VALUE;
    const auto& obj = getObject();
    auto        it  = obj.find(key);
    return it != obj.end() ? it->second : NULL_VALUE;
}

const JsonValue& JsonValue::operator[](size_t index) const
{
    if (!isArray())
        return NULL_VALUE;
    const auto& arr = getArray();
    return index < arr.size() ? arr[index] : NULL_VALUE;
}

JsonValue JsonValue::parse(JsonParser& parser)
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

JsonValue::Array JsonValue::parseArray(JsonParser& parser)
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

JsonValue::Object JsonValue::parseObject(JsonParser& parser)
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