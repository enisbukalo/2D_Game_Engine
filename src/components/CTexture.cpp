#include "CTexture.h"

namespace Components
{

std::string CTexture::getTexturePath() const
{
    return texturePath;
}

void CTexture::setTexturePath(const std::string& path)
{
    texturePath = path;
}

}  // namespace Components
