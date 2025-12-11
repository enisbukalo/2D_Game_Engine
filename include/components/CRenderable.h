#ifndef CRENDERABLE_H
#define CRENDERABLE_H

#include <string>
#include "Color.h"
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
struct CRenderable
{
    CRenderable() = default;
    CRenderable(VisualType visualType, const Color& color = Color::White, int zIndex = 0, bool visible = true)
        : visualType(visualType), color(color), zIndex(zIndex), visible(visible)
    {
    }

    VisualType getVisualType() const;
    void       setVisualType(VisualType visualType);

    Color getColor() const;
    void  setColor(const Color& color);

    int  getZIndex() const;
    void setZIndex(int zIndex);

    bool isVisible() const;
    void setVisible(bool visible);

    Vec2  getLineStart() const;
    void  setLineStart(const Vec2& start);
    Vec2  getLineEnd() const;
    void  setLineEnd(const Vec2& end);
    float getLineThickness() const;
    void  setLineThickness(float thickness);

    VisualType visualType = VisualType::None;
    Color      color      = Color::White;
    int        zIndex     = 0;
    bool       visible    = true;

    Vec2  lineStart     = Vec2(0.0f, 0.0f);
    Vec2  lineEnd       = Vec2(1.0f, 0.0f);
    float lineThickness = 2.0f;
};

}  // namespace Components

#endif  // CRENDERABLE_H
