#include "CRenderable.h"
#include "JsonBuilder.h"
#include "JsonParser.h"

std::string CRenderable::getType() const
{
    return "Renderable";
}

void CRenderable::serialize(JsonBuilder& builder) const
{
    builder.beginObject();
    builder.addKey("cRenderable");
    builder.beginObject();

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

    builder.endObject();
    builder.endObject();
}

void CRenderable::deserialize(const JsonValue& value)
{
    const auto& renderable = value["cRenderable"];

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
