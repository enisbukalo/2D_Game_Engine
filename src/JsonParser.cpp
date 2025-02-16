#include "JsonParser.h"
#include <cctype>
#include <cmath>

bool JsonParser::hasNext() const
{
    return m_pos < m_json.length();
}

void JsonParser::beginObject()
{
    skipWhitespace();
    if (!hasNext() || m_json[m_pos++] != '{')
        throw std::runtime_error("Expected '{'");
}

void JsonParser::endObject()
{
    skipWhitespace();
    if (!hasNext() || m_json[m_pos++] != '}')
        throw std::runtime_error("Expected '}'");
}

void JsonParser::beginArray()
{
    skipWhitespace();
    if (!hasNext() || m_json[m_pos++] != '[')
        throw std::runtime_error("Expected '['");
}

void JsonParser::endArray()
{
    skipWhitespace();
    if (!hasNext() || m_json[m_pos++] != ']')
        throw std::runtime_error("Expected ']'");
}

std::string JsonParser::getKey()
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

std::string JsonParser::getString()
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

float JsonParser::getNumber()
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

bool JsonParser::getBool()
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

void JsonParser::getNull()
{
    skipWhitespace();
    if (m_pos + 4 > m_json.length() || m_json.substr(m_pos, 4) != "null")
    {
        throw std::runtime_error("Expected 'null'");
    }
    m_pos += 4;
}

char JsonParser::peek()
{
    skipWhitespace();
    return m_pos < m_json.length() ? m_json[m_pos] : '\0';
}

void JsonParser::consume()
{
    if (m_pos < m_json.length())
    {
        m_pos++;
    }
}

void JsonParser::skipWhitespace()
{
    while (m_pos < m_json.length() && std::isspace(m_json[m_pos]))
    {
        m_pos++;
    }
}