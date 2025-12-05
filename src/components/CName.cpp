#include "CName.h"

std::string CName::getType() const
{
    return "Name";
}

void CName::serialize(JsonBuilder& builder) const
{
    builder.beginObject();
    builder.addKey("cName");
    builder.beginObject();
    builder.addKey("guid");
    builder.addString(getGuid());
    builder.addKey("name");
    builder.addString(m_name);
    builder.endObject();
    builder.endObject();
}

void CName::deserialize(const JsonValue& value)
{
    const auto& nameComp = value["cName"];
    if (nameComp.hasKey("guid"))
    {
        setGuid(nameComp["guid"].getString());
    }
    m_name = nameComp["name"].getString();
}

const std::string& CName::getName() const
{
    return m_name;
}

void CName::setName(const std::string& name)
{
    m_name = name;
}
