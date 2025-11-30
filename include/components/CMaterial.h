#ifndef CMATERIAL_H
#define CMATERIAL_H

#include "Component.h"
#include "Color.h"
#include <string>

/**
 * @brief Enumeration of blend modes for rendering
 */
enum class BlendMode
{
    Alpha,      ///< Standard alpha blending (src.a, 1-src.a)
    Add,        ///< Additive blending (src, 1)
    Multiply,   ///< Multiplicative blending (dst, 0)
    None        ///< No blending (replace)
};

/**
 * @brief Component for material properties
 *
 * @description
 * CMaterial defines the visual appearance of an entity by combining texture,
 * shader, color tint, blend mode, and opacity. It serves as a reference to
 * texture and shader resources while storing immediate material properties.
 * The rendering system uses this component to apply the correct visual
 * styling to entities.
 */
struct CMaterial : public Component
{
public:
    /** @brief Default constructor */
    CMaterial() = default;

    /**
     * @brief Constructs a material with specified properties
     * @param textureGuid GUID of the texture component (empty for no texture)
     * @param shaderGuid GUID of the shader component (empty for no shader)
     * @param tint Color tint/modulation
     * @param blendMode Blending mode
     * @param opacity Opacity/alpha multiplier (0.0 = transparent, 1.0 = opaque)
     */
    CMaterial(const std::string& textureGuid, const std::string& shaderGuid, const Color& tint = Color::White,
              BlendMode blendMode = BlendMode::Alpha, float opacity = 1.0f)
        : m_textureGuid(textureGuid), m_shaderGuid(shaderGuid), m_tint(tint), m_blendMode(blendMode), m_opacity(opacity)
    {
    }

    /**
     * @brief Gets the type identifier for this component
     * @return String "Material"
     */
    std::string getType() const override;

    /**
     * @brief Serializes the material data
     */
    void serialize(JsonBuilder& builder) const override;

    /**
     * @brief Deserializes material data
     */
    void deserialize(const JsonValue& value) override;

    /**
     * @brief Gets the texture component GUID
     * @return GUID of texture component
     */
    std::string getTextureGuid() const;

    /**
     * @brief Sets the texture component GUID
     * @param textureGuid GUID of texture component
     */
    void setTextureGuid(const std::string& textureGuid);

    /**
     * @brief Gets the shader component GUID
     * @return GUID of shader component
     */
    std::string getShaderGuid() const;

    /**
     * @brief Sets the shader component GUID
     * @param shaderGuid GUID of shader component
     */
    void setShaderGuid(const std::string& shaderGuid);

    /**
     * @brief Gets the color tint
     * @return Current color tint
     */
    Color getTint() const;

    /**
     * @brief Sets the color tint
     * @param tint New color tint
     */
    void setTint(const Color& tint);

    /**
     * @brief Gets the blend mode
     * @return Current blend mode
     */
    BlendMode getBlendMode() const;

    /**
     * @brief Sets the blend mode
     * @param blendMode New blend mode
     */
    void setBlendMode(BlendMode blendMode);

    /**
     * @brief Gets the opacity
     * @return Current opacity (0.0-1.0)
     */
    float getOpacity() const;

    /**
     * @brief Sets the opacity
     * @param opacity New opacity (0.0-1.0)
     */
    void setOpacity(float opacity);

private:
    std::string m_textureGuid;                 ///< GUID of texture component reference
    std::string m_shaderGuid;                  ///< GUID of shader component reference
    Color       m_tint      = Color::White;    ///< Color tint/modulation
    BlendMode   m_blendMode = BlendMode::Alpha;  ///< Blending mode
    float       m_opacity   = 1.0f;            ///< Opacity multiplier (0.0-1.0)
};

#endif  // CMATERIAL_H
