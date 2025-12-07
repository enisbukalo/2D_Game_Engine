#include "JsonBuilder.h"

namespace Serialization {

JsonBuilder::JsonBuilder() : m_needsComma(false) {}

void JsonBuilder::beginObject()
{
    if (m_needsComma)
        m_stream << ",";
    m_stream << "{";
    m_needsComma = false;
}

void JsonBuilder::endObject()
{
    m_stream << "}";
    m_needsComma = true;
}

void JsonBuilder::beginArray()
{
    if (m_needsComma)
        m_stream << ",";
    m_stream << "[";
    m_needsComma = false;
}

void JsonBuilder::endArray()
{
    m_stream << "]";
    m_needsComma = true;
}

void JsonBuilder::addKey(const std::string& key)
{
    if (m_needsComma)
        m_stream << ",";
    m_stream << "\"" << escapeString(key) << "\":";
    m_needsComma = false;
}

void JsonBuilder::addString(const std::string& value)
{
    if (m_needsComma)
        m_stream << ",";
    m_stream << "\"" << escapeString(value) << "\"";
    m_needsComma = true;
}

void JsonBuilder::addNumber(float value)
{
    if (m_needsComma)
        m_stream << ",";
    m_stream << value;
    m_needsComma = true;
}

void JsonBuilder::addBool(bool value)
{
    if (m_needsComma)
        m_stream << ",";
    m_stream << (value ? "true" : "false");
    m_needsComma = true;
}

std::string JsonBuilder::toString() const
{
    return m_stream.str();
}

std::string JsonBuilder::escapeString(const std::string& str)
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

} // namespace Serialization
