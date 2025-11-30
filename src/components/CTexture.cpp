#include "CTexture.h"
#include "JsonBuilder.h"
#include "JsonParser.h"

std::string CTexture::getType() const
{
    return "Texture";
}

void CTexture::serialize(JsonBuilder& builder) const
{
    builder.beginObject();
    builder.addKey("cTexture");
    builder.beginObject();
    builder.addKey("texturePath");
    builder.addString(m_texturePath);
    builder.endObject();
    builder.endObject();
}

void CTexture::deserialize(const JsonValue& value)
{
    const auto& texture = value["cTexture"];
    m_texturePath       = texture["texturePath"].getString();
}

std::string CTexture::getTexturePath() const
{
    return m_texturePath;
}

void CTexture::setTexturePath(const std::string& texturePath)
{
    m_texturePath = texturePath;
}
