#ifndef CTEXTURE_H
#define CTEXTURE_H

#include <string>

namespace Components
{

/**
 * @brief Component for texture resources
 *
 * @description
 * CTexture stores texture resource information for entities. It maintains the
 * path to the texture file and acts as a reference for the rendering system.
 * The actual texture loading and caching is handled by the rendering system
 * to avoid duplicate texture loads and improve performance.
 */
struct CTexture
{
    CTexture() = default;
    explicit CTexture(const std::string& texturePath) : texturePath(texturePath) {}

    inline std::string getTexturePath() const
    {
        return texturePath;
    }
    inline void setTexturePath(const std::string& newTexturePath)
    {
        texturePath = newTexturePath;
    }

    std::string texturePath;
};

}  // namespace Components

#endif  // CTEXTURE_H
