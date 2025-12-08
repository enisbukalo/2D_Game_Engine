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

    // Serialize component GUID
    builder.addKey("guid");
    builder.addString(getGuid());

    // Serialize visual type
    builder.addKey("visualType");
    builder.addNumber(static_cast<int>(m_visualType));

    // Serialize color
    builder.addKey("color");
    builder.beginObject();
    builder.addKey("r");
    builder.addNumber(m_color.r);
    builder.addKey("g");
    builder.addNumber(m_color.g);
    builder.addKey("b");
    builder.addNumber(m_color.b);
    builder.addKey("a");
    builder.addNumber(m_color.a);
    builder.endObject();

    // Serialize z-index
    builder.addKey("zIndex");
    builder.addNumber(m_zIndex);

    // Serialize visibility
    builder.addKey("visible");
    builder.addBool(m_visible);

    // Serialize line properties (for Line visual type)
    if (m_visualType == VisualType::Line)
    {
        builder.addKey("lineStart");
        builder.beginObject();
        builder.addKey("x");
        builder.addNumber(m_lineStart.x);
        builder.addKey("y");
        builder.addNumber(m_lineStart.y);
        builder.endObject();

        builder.addKey("lineEnd");
        builder.beginObject();
        builder.addKey("x");
        builder.addNumber(m_lineEnd.x);
        builder.addKey("y");
        builder.addNumber(m_lineEnd.y);
        builder.endObject();

        builder.addKey("lineThickness");
        builder.addNumber(m_lineThickness);
    }

    builder.endObject();
    builder.endObject();
}

void CRenderable::deserialize(const Serialization::SSerialization::JsonValue& value)
{
    const auto& renderable = value["cRenderable"];

    // Deserialize component GUID
    if (renderable.hasKey("guid"))
    {
        setGuid(renderable["guid"].getString());
    }

    // Deserialize visual type
    m_visualType = static_cast<VisualType>(static_cast<int>(renderable["visualType"].getNumber()));

    // Deserialize color
    const auto& color = renderable["color"];
    m_color.r         = static_cast<uint8_t>(color["r"].getNumber());
    m_color.g         = static_cast<uint8_t>(color["g"].getNumber());
    m_color.b         = static_cast<uint8_t>(color["b"].getNumber());
    m_color.a         = static_cast<uint8_t>(color["a"].getNumber());

    // Deserialize z-index
    m_zIndex = static_cast<int>(renderable["zIndex"].getNumber());

    // Deserialize visibility
    m_visible = renderable["visible"].getBool();

    // Deserialize line properties (for Line visual type)
    if (m_visualType == VisualType::Line && renderable.hasKey("lineStart"))
    {
        const auto& lineStart = renderable["lineStart"];
        m_lineStart.x         = static_cast<float>(lineStart["x"].getNumber());
        m_lineStart.y         = static_cast<float>(lineStart["y"].getNumber());

        const auto& lineEnd = renderable["lineEnd"];
        m_lineEnd.x         = static_cast<float>(lineEnd["x"].getNumber());
        m_lineEnd.y         = static_cast<float>(lineEnd["y"].getNumber());

        m_lineThickness = static_cast<float>(renderable["lineThickness"].getNumber());
    }
}

VisualType CRenderable::getVisualType() const
{
    return m_visualType;
}

void CRenderable::setVisualType(VisualType visualType)
{
    m_visualType = visualType;
}

Color CRenderable::getColor() const
{
    return m_color;
}

void CRenderable::setColor(const Color& color)
{
    m_color = color;
}

int CRenderable::getZIndex() const
{
    return m_zIndex;
}

void CRenderable::setZIndex(int zIndex)
{
    m_zIndex = zIndex;
}

bool CRenderable::isVisible() const
{
    return m_visible;
}

void CRenderable::setVisible(bool visible)
{
    m_visible = visible;
}

Vec2 CRenderable::getLineStart() const
{
    return m_lineStart;
}

void CRenderable::setLineStart(const Vec2& start)
{
    m_lineStart = start;
}

Vec2 CRenderable::getLineEnd() const
{
    return m_lineEnd;
}

void CRenderable::setLineEnd(const Vec2& end)
{
    m_lineEnd = end;
}

float CRenderable::getLineThickness() const
{
    return m_lineThickness;
}

void CRenderable::setLineThickness(float thickness)
{
    m_lineThickness = thickness;
}

}  // namespace Components
