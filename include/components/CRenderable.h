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

    inline VisualType getVisualType() const { return visualType; }
    inline void setVisualType(VisualType newVisualType) { visualType = newVisualType; }

    inline Color getColor() const { return color; }
    inline void setColor(const Color& newColor) { color = newColor; }

    inline int getZIndex() const { return zIndex; }
    inline void setZIndex(int newZIndex) { zIndex = newZIndex; }

    inline bool isVisible() const { return visible; }
    inline void setVisible(bool isVisible) { visible = isVisible; }

    inline Vec2 getLineStart() const { return lineStart; }
    inline void setLineStart(const Vec2& start) { lineStart = start; }
    inline Vec2 getLineEnd() const { return lineEnd; }
    inline void setLineEnd(const Vec2& end) { lineEnd = end; }
    inline float getLineThickness() const { return lineThickness; }
    inline void setLineThickness(float thickness) { lineThickness = thickness; }

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
