#include "CTexture.h"
#include "SSerialization.h"

namespace Components
{

std::string CTexture::getType() const
{
    return "Texture";
}

void CTexture::serialize(Serialization::JsonBuilder& builder) const
{
    builder.beginObject();
    builder.addKey("cTexture");
    builder.beginObject();
    builder.addKey("texturePath");
    builder.addString(texturePath);
    builder.endObject();
    builder.endObject();
}

void CTexture::deserialize(const Serialization::SSerialization::JsonValue& value)
{
    const auto& texture = value["cTexture"];
    texturePath = texture["texturePath"].getString();
}

std::string CTexture::getTexturePath() const
{
    return texturePath;
}

void CTexture::setTexturePath(const std::string& texturePath)
{
    this->texturePath = texturePath;
}

}  // namespace Components
