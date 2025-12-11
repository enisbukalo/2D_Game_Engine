#ifndef CTEXTURE_H
#define CTEXTURE_H

#include <string>
#include "SSerialization.h"

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

    std::string getType() const;
    void        serialize(Serialization::JsonBuilder& builder) const;
    void        deserialize(const Serialization::SSerialization::JsonValue& value);

    std::string getTexturePath() const;
    void        setTexturePath(const std::string& texturePath);

    std::string texturePath;
};

}  // namespace Components

#endif  // CTEXTURE_H
