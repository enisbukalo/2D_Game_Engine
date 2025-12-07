#ifndef CTEXTURE_H
#define CTEXTURE_H

#include <string>
#include "Component.h"

/**
 * @brief Component for texture resources
 *
 * @description
 * CTexture stores texture resource information for entities. It maintains the
 * path to the texture file and acts as a reference for the rendering system.
 * The actual texture loading and caching is handled by the rendering system
 * to avoid duplicate texture loads and improve performance.
 */
struct CTexture : public Component
{
public:
    /** @brief Default constructor */
    CTexture() = default;

    /**
     * @brief Constructs a texture component with a file path
     * @param texturePath Path to the texture file
     */
    explicit CTexture(const std::string& texturePath) : m_texturePath(texturePath) {}

    /**
     * @brief Gets the type identifier for this component
     * @return String "Texture"
     */
    std::string getType() const override;

    /**
     * @brief Serializes the texture data
     */
    void serialize(Serialization::JsonBuilder& builder) const override;

    /**
     * @brief Deserializes texture data from JSON value
     */
    void deserialize(const Serialization::SSerialization::JsonValue& value) override;

    /**
     * @brief Gets the texture file path
     * @return Path to texture file
     */
    std::string getTexturePath() const;

    /**
     * @brief Sets the texture file path
     * @param texturePath New path to texture file
     */
    void setTexturePath(const std::string& texturePath);

private:
    std::string m_texturePath;  ///< Path to the texture file
};

#endif  // CTEXTURE_H
