#ifndef CRENDERABLE_H
#define CRENDERABLE_H

#include <string>
#include "Color.h"
#include "Component.h"
#include "Vec2.h"

namespace Components
{

/**
 * @brief Enumeration of visual types for rendering
 */
enum class VisualType
{
    None,       ///< No visual (invisible)
    Rectangle,  ///< Rectangle shape
    Circle,     ///< Circle shape
    Sprite,     ///< Textured sprite
    Line,       ///< Line primitive
    Custom      ///< Custom rendering (via shader/material)
};

/**
 * @brief Component for rendering visual representation of entities
 *
 * @description
 * CRenderable defines how an entity should be visually represented in the game.
 * It includes properties like visual type, color, z-index for layer ordering,
 * and visibility control. This component is used by the rendering system to
 * determine what and how to draw each entity.
 */
struct CRenderable : public Component
{
public:
    /** @brief Default constructor */
    CRenderable() = default;

    /**
     * @brief Constructs a renderable with specified properties
     * @param visualType Type of visual representation
     * @param color Base color/tint
     * @param zIndex Layer ordering (higher values draw on top)
     * @param visible Initial visibility state
     */
    CRenderable(VisualType visualType, const Color& color = Color::White, int zIndex = 0, bool visible = true)
        : m_visualType(visualType), m_color(color), m_zIndex(zIndex), m_visible(visible)
    {
    }

    /**
     * @brief Gets the type identifier for this component
     * @return String "Renderable"
     */
    std::string getType() const override;

    /**
     * @brief Serializes the renderable data
     */
    void serialize(Serialization::JsonBuilder& builder) const override;

    /**
     * @brief Deserializes renderable data from JSON value
     */
    void deserialize(const Serialization::SSerialization::JsonValue& value) override;

    /**
     * @brief Gets the visual type
     * @return Current visual type
     */
    VisualType getVisualType() const;

    /**
     * @brief Sets the visual type
     * @param visualType New visual type
     */
    void setVisualType(VisualType visualType);

    /**
     * @brief Gets the base color/tint
     * @return Current color
     */
    Color getColor() const;

    /**
     * @brief Sets the base color/tint
     * @param color New color
     */
    void setColor(const Color& color);

    /**
     * @brief Gets the z-index (layer order)
     * @return Current z-index
     */
    int getZIndex() const;

    /**
     * @brief Sets the z-index (layer order)
     * @param zIndex New z-index
     */
    void setZIndex(int zIndex);

    /**
     * @brief Checks if the entity is visible
     * @return true if visible, false otherwise
     */
    bool isVisible() const;

    /**
     * @brief Sets the visibility state
     * @param visible New visibility state
     */
    void setVisible(bool visible);

    /**
     * @brief Gets the line start point (local space)
     * @return Start point coordinates
     */
    Vec2 getLineStart() const;

    /**
     * @brief Sets the line start point (local space)
     * @param start Start point coordinates
     */
    void setLineStart(const Vec2& start);

    /**
     * @brief Gets the line end point (local space)
     * @return End point coordinates
     */
    Vec2 getLineEnd() const;

    /**
     * @brief Sets the line end point (local space)
     * @param end End point coordinates
     */
    void setLineEnd(const Vec2& end);

    /**
     * @brief Gets the line thickness in pixels
     * @return Line thickness
     */
    float getLineThickness() const;

    /**
     * @brief Sets the line thickness in pixels
     * @param thickness Line thickness
     */
    void setLineThickness(float thickness);

private:
    VisualType m_visualType = VisualType::None;  ///< Type of visual representation
    Color      m_color      = Color::White;      ///< Base color/tint
    int        m_zIndex     = 0;                 ///< Layer ordering (higher = on top)
    bool       m_visible    = true;              ///< Visibility flag

    // Line-specific properties (used when m_visualType == Line)
    Vec2  m_lineStart     = Vec2(0.0f, 0.0f);  ///< Line start point (local space)
    Vec2  m_lineEnd       = Vec2(1.0f, 0.0f);  ///< Line end point (local space)
    float m_lineThickness = 2.0f;              ///< Line thickness in pixels
};

}  // namespace Components

#endif  // CRENDERABLE_H
