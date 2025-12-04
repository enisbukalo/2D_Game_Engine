#include "CMaterial.h"
#include "JsonBuilder.h"
#include "JsonParser.h"

std::string CMaterial::getType() const
{
    return "Material";
}

void CMaterial::serialize(JsonBuilder& builder) const
{
    builder.beginObject();
    builder.addKey("cMaterial");
    builder.beginObject();

    // Serialize component GUID
    builder.addKey("guid");
    builder.addString(getGuid());

    // Serialize texture GUID
    builder.addKey("textureGuid");
    builder.addString(m_textureGuid);

    // Serialize shader GUID
    builder.addKey("shaderGuid");
    builder.addString(m_shaderGuid);

    // Serialize tint color
    builder.addKey("tint");
    builder.beginObject();
    builder.addKey("r");
    builder.addNumber(m_tint.r);
    builder.addKey("g");
    builder.addNumber(m_tint.g);
    builder.addKey("b");
    builder.addNumber(m_tint.b);
    builder.addKey("a");
    builder.addNumber(m_tint.a);
    builder.endObject();

    // Serialize blend mode
    builder.addKey("blendMode");
    builder.addNumber(static_cast<int>(m_blendMode));

    // Serialize opacity
    builder.addKey("opacity");
    builder.addNumber(m_opacity);

    builder.endObject();
    builder.endObject();
}

void CMaterial::deserialize(const JsonValue& value)
{
    const auto& material = value["cMaterial"];

    // Deserialize component GUID
    if (material.hasKey("guid"))
    {
        setGuid(material["guid"].getString());
    }

    // Deserialize texture GUID
    m_textureGuid = material["textureGuid"].getString();

    // Deserialize shader GUID
    m_shaderGuid = material["shaderGuid"].getString();

    // Deserialize tint color
    const auto& tint = material["tint"];
    m_tint.r         = static_cast<uint8_t>(tint["r"].getNumber());
    m_tint.g         = static_cast<uint8_t>(tint["g"].getNumber());
    m_tint.b         = static_cast<uint8_t>(tint["b"].getNumber());
    m_tint.a         = static_cast<uint8_t>(tint["a"].getNumber());

    // Deserialize blend mode
    m_blendMode = static_cast<BlendMode>(static_cast<int>(material["blendMode"].getNumber()));

    // Deserialize opacity
    m_opacity = static_cast<float>(material["opacity"].getNumber());
}

std::string CMaterial::getTextureGuid() const
{
    return m_textureGuid;
}

void CMaterial::setTextureGuid(const std::string& textureGuid)
{
    m_textureGuid = textureGuid;
}

std::string CMaterial::getShaderGuid() const
{
    return m_shaderGuid;
}

void CMaterial::setShaderGuid(const std::string& shaderGuid)
{
    m_shaderGuid = shaderGuid;
}

Color CMaterial::getTint() const
{
    return m_tint;
}

void CMaterial::setTint(const Color& tint)
{
    m_tint = tint;
}

BlendMode CMaterial::getBlendMode() const
{
    return m_blendMode;
}

void CMaterial::setBlendMode(BlendMode blendMode)
{
    m_blendMode = blendMode;
}

float CMaterial::getOpacity() const
{
    return m_opacity;
}

void CMaterial::setOpacity(float opacity)
{
    m_opacity = opacity;
}
