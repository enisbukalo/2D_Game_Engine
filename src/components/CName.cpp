#include "CName.h"

namespace Components
{

std::string CName::getType() const
{
    return "Name";
}

void CName::serialize(Serialization::JsonBuilder& builder) const
{
    builder.beginObject();
    builder.addKey("cName");
    builder.beginObject();
    builder.addKey("name");
    builder.addString(name);
    builder.endObject();
    builder.endObject();
}

void CName::deserialize(const Serialization::SSerialization::JsonValue& value)
{
    const auto& nameComp = value["cName"];
    name = nameComp["name"].getString();
}

const std::string& CName::getName() const
{
    return name;
}

void CName::setName(const std::string& name)
{
    this->name = name;
}

}  // namespace Components
