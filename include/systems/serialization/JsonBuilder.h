#ifndef JSONBUILDER_H
#define JSONBUILDER_H

#include <sstream>
#include <string>

namespace Serialization {

class JsonBuilder
{
public:
    JsonBuilder();

    void beginObject();
    void endObject();
    void beginArray();
    void endArray();
    void addKey(const std::string& key);
    void addString(const std::string& value);
    void addNumber(float value);
    void addBool(bool value);

    std::string toString() const;

private:
    std::string escapeString(const std::string& str);

    std::stringstream m_stream;      ///< Stream for building the JSON string
    bool              m_needsComma;  ///< Flag indicating if a comma is needed before next value
};

} // namespace Serialization

#endif // JSONBUILDER_H
