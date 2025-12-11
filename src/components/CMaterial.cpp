#include "CMaterial.h"

namespace Components
{

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
