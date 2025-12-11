#include "CRenderable.h"
#include "SSerialization.h"

namespace Components
{

std::string CRenderable::getType() const
{
    return "Renderable";
}

void CRenderable::serialize(Serialization::JsonBuilder& builder) const
{
    builder.beginObject();
    builder.addKey("cRenderable");
    builder.beginObject();
    builder.addKey("visualType");
    builder.addNumber(static_cast<int>(visualType));

    builder.addKey("color");
    builder.beginObject();
    builder.addKey("r");
    builder.addNumber(color.r);
    builder.addKey("g");
    builder.addNumber(color.g);
    builder.addKey("b");
    builder.addNumber(color.b);
    builder.addKey("a");
    builder.addNumber(color.a);
    builder.endObject();
    builder.addKey("zIndex");
    builder.addNumber(zIndex);

    builder.addKey("visible");
    builder.addBool(visible);

    if (visualType == VisualType::Line)
    {
        builder.addKey("lineStart");
        builder.beginObject();
        builder.addKey("x");
        builder.addNumber(lineStart.x);
        builder.addKey("y");
        builder.addNumber(lineStart.y);
        builder.endObject();

        builder.addKey("lineEnd");
        builder.beginObject();
        builder.addKey("x");
        builder.addNumber(lineEnd.x);
        builder.addKey("y");
        builder.addNumber(lineEnd.y);
        builder.endObject();

        builder.addKey("lineThickness");
        builder.addNumber(lineThickness);
    }

    builder.endObject();
    builder.endObject();
}

void CRenderable::deserialize(const Serialization::SSerialization::JsonValue& value)
{
    const auto& renderable = value["cRenderable"];
    visualType = static_cast<VisualType>(static_cast<int>(renderable["visualType"].getNumber()));

    const auto& color = renderable["color"];
    this->color.r    = static_cast<uint8_t>(color["r"].getNumber());
    this->color.g    = static_cast<uint8_t>(color["g"].getNumber());
    this->color.b    = static_cast<uint8_t>(color["b"].getNumber());
    this->color.a    = static_cast<uint8_t>(color["a"].getNumber());

    zIndex  = static_cast<int>(renderable["zIndex"].getNumber());
    visible = renderable["visible"].getBool();

    if (visualType == VisualType::Line && renderable.hasKey("lineStart"))
    {
        const auto& lineStart = renderable["lineStart"];
        this->lineStart.x     = static_cast<float>(lineStart["x"].getNumber());
        this->lineStart.y     = static_cast<float>(lineStart["y"].getNumber());

        const auto& lineEnd = renderable["lineEnd"];
        this->lineEnd.x     = static_cast<float>(lineEnd["x"].getNumber());
        this->lineEnd.y     = static_cast<float>(lineEnd["y"].getNumber());

        lineThickness = static_cast<float>(renderable["lineThickness"].getNumber());
    }
}

VisualType CRenderable::getVisualType() const
{
    return visualType;
}

void CRenderable::setVisualType(VisualType visualType)
{
    this->visualType = visualType;
}

Color CRenderable::getColor() const
{
    return color;
}

void CRenderable::setColor(const Color& color)
{
    this->color = color;
}

int CRenderable::getZIndex() const
{
    return zIndex;
}

void CRenderable::setZIndex(int zIndex)
{
    this->zIndex = zIndex;
}

bool CRenderable::isVisible() const
{
    return visible;
}

void CRenderable::setVisible(bool visible)
{
    this->visible = visible;
}

Vec2 CRenderable::getLineStart() const
{
    return lineStart;
}

void CRenderable::setLineStart(const Vec2& start)
{
    lineStart = start;
}

Vec2 CRenderable::getLineEnd() const
{
    return lineEnd;
}

void CRenderable::setLineEnd(const Vec2& end)
{
    lineEnd = end;
}

float CRenderable::getLineThickness() const
{
    return lineThickness;
}

void CRenderable::setLineThickness(float thickness)
{
    lineThickness = thickness;
}

}  // namespace Components
