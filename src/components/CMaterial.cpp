#include "CMaterial.h"
#include "SSerialization.h"

namespace Components
{

std::string CMaterial::getType() const
{
    return "Material";
}

void CMaterial::serialize(Serialization::JsonBuilder& builder) const
{
    builder.beginObject();
    builder.addKey("cMaterial");
    builder.beginObject();
    builder.addKey("tint");
    builder.beginObject();
    builder.addKey("r");
    builder.addNumber(tint.r);
    builder.addKey("g");
    builder.addNumber(tint.g);
    builder.addKey("b");
    builder.addNumber(tint.b);
    builder.addKey("a");
    builder.addNumber(tint.a);
    builder.endObject();
    builder.addKey("blendMode");
    builder.addNumber(static_cast<int>(blendMode));
    builder.addKey("opacity");
    builder.addNumber(opacity);

    builder.endObject();
    builder.endObject();
}

void CMaterial::deserialize(const Serialization::SSerialization::JsonValue& value)
{
    const auto& material = value["cMaterial"];
    const auto& tint = material["tint"];
    tint.r              = static_cast<uint8_t>(tintObj["r"].getNumber());
    tint.g              = static_cast<uint8_t>(tintObj["g"].getNumber());
    tint.b              = static_cast<uint8_t>(tintObj["b"].getNumber());
    tint.a              = static_cast<uint8_t>(tintObj["a"].getNumber());

    blendMode = static_cast<BlendMode>(static_cast<int>(material["blendMode"].getNumber()));
    opacity   = static_cast<float>(material["opacity"].getNumber());
}

Color CMaterial::getTint() const
{
    return tint;
}

void CMaterial::setTint(const Color& tint)
{
    this->tint = tint;
}

BlendMode CMaterial::getBlendMode() const
{
    return blendMode;
}

void CMaterial::setBlendMode(BlendMode blendMode)
{
    this->blendMode = blendMode;
}

float CMaterial::getOpacity() const
{
    return opacity;
}

void CMaterial::setOpacity(float opacity)
{
    this->opacity = opacity;
}

}  // namespace Components
