#include "CTexture.h"
#include "SSerialization.h"

std::string CTexture::getType() const
{
    return "Texture";
}

void CTexture::serialize(Serialization::JsonBuilder& builder) const
{
    builder.beginObject();
    builder.addKey("cTexture");
    builder.beginObject();
    builder.addKey("guid");
    builder.addString(getGuid());
    builder.addKey("texturePath");
    builder.addString(m_texturePath);
    builder.endObject();
    builder.endObject();
}

void CTexture::deserialize(const Serialization::SSerialization::JsonValue& value)
{
    const auto& texture = value["cTexture"];
    if (texture.hasKey("guid"))
    {
        setGuid(texture["guid"].getString());
    }
    m_texturePath = texture["texturePath"].getString();
}

std::string CTexture::getTexturePath() const
{
    return m_texturePath;
}

void CTexture::setTexturePath(const std::string& texturePath)
{
    m_texturePath = texturePath;
}
