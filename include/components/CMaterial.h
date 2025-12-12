#ifndef CMATERIAL_H
#define CMATERIAL_H

#include <string>
#include "Color.h"

namespace Components
{

/**
 * @brief Enumeration of blend modes for rendering
 */
enum class BlendMode
{
    Alpha,     ///< Standard alpha blending (src.a, 1-src.a)
    Add,       ///< Additive blending (src, 1)
    Multiply,  ///< Multiplicative blending (dst, 0)
    None       ///< No blending (replace)
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
struct CMaterial
{
    CMaterial() = default;
    CMaterial(const Color& tint, BlendMode blendMode = BlendMode::Alpha, float opacity = 1.0f)
        : tint(tint), blendMode(blendMode), opacity(opacity)
    {
    }

    inline Color getTint() const { return tint; }
    inline void setTint(const Color& newTint) { tint = newTint; }
    inline BlendMode getBlendMode() const { return blendMode; }
    inline void setBlendMode(BlendMode newBlendMode) { blendMode = newBlendMode; }
    inline float getOpacity() const { return opacity; }
    inline void setOpacity(float newOpacity) { opacity = newOpacity; }

    Color     tint      = Color::White;
    BlendMode blendMode = BlendMode::Alpha;
    float     opacity   = 1.0f;
};

}  // namespace Components

#endif  // CMATERIAL_H
