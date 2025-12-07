#ifndef JSONPARSER_H
#define JSONPARSER_H

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace Serialization {

class JsonParser
{
public:
    explicit JsonParser(const std::string& json) : m_json(json), m_pos(0) {}

    static JsonParser fromFile(const std::string& path);

    bool        hasNext() const;
    void        beginObject();
    void        endObject();
    void        beginArray();
    void        endArray();
    std::string getKey();
    std::string getString();
    float       getNumber();
    bool        getBool();
    void        getNull();
    char        peek();
    void        consume();
    void        skipWhitespace();

private:
    std::string m_json;
    size_t      m_pos;
};

} // namespace Serialization

#endif // JSONPARSER_H
