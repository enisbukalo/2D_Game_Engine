#include "CRenderable.h"

namespace Components
{

VisualType CRenderable::getVisualType() const
{
    return visualType;
}

void CRenderable::setVisualType(VisualType newVisualType)
{
    visualType = newVisualType;
}

Color CRenderable::getColor() const
{
    return color;
}

void CRenderable::setColor(const Color& newColor)
{
    color = newColor;
}

int CRenderable::getZIndex() const
{
    return zIndex;
}

void CRenderable::setZIndex(int newZIndex)
{
    zIndex = newZIndex;
}

bool CRenderable::isVisible() const
{
    return visible;
}

void CRenderable::setVisible(bool isVisible)
{
    visible = isVisible;
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
