#ifndef AUDIOTYPES_H
#define AUDIOTYPES_H

#include <cstdint>
#include <functional>
#include <limits>

/**
 * @brief Audio system type definitions and constants
 *
 * @description
 * This header defines common types, enums, and constants used by the audio system.
 * It provides type-safe handles for audio playback and categorization of audio types.
 */

/**
 * @brief Type of audio resource
 */
enum class AudioType
{
    SFX,   ///< Sound effects - loaded into memory, pooled playback
    Music  ///< Music - streamed from disk
};

/**
 * @brief Fade curve type for volume transitions
 */
enum class FadeCurve
{
    Linear,    ///< Linear interpolation
    EaseIn,    ///< Slow start, fast end (quadratic)
    EaseOut,   ///< Fast start, slow end (quadratic)
    EaseInOut  ///< Slow start and end, fast middle (smoothstep)
};

/**
 * @brief Fade state for tracking volume transitions
 */
enum class FadeState
{
    None,       ///< Not fading
    FadingIn,   ///< Fading in (volume increasing)
    FadingOut   ///< Fading out (volume decreasing)
};

/**
 * @brief Handle to an active sound instance in the SFX pool
 *
 * @description
 * AudioHandle uses a generation counter to safely identify sound instances
 * even after pool slots are reused. Invalid handles have index set to INVALID_INDEX.
 */
struct AudioHandle
{
    static constexpr uint32_t INVALID_INDEX = std::numeric_limits<uint32_t>::max();

    uint32_t index      = INVALID_INDEX;  ///< Index in the sound pool
    uint32_t generation = 0;              ///< Generation counter for handle validation

    /**
     * @brief Check if the handle is valid
     * @return true if handle points to a potentially valid sound
     */
    bool isValid() const
    {
        return index != INVALID_INDEX;
    }

    /**
     * @brief Create an invalid handle
     * @return AudioHandle with invalid index
     */
    static AudioHandle invalid()
    {
        return AudioHandle{INVALID_INDEX, 0};
    }
};

/**
 * @brief Audio system constants
 */
namespace AudioConstants
{
constexpr size_t DEFAULT_SFX_POOL_SIZE = 32;  ///< Default number of simultaneous sound effects
constexpr float  DEFAULT_MASTER_VOLUME = 1.0f;
constexpr float  DEFAULT_SFX_VOLUME    = 1.0f;
constexpr float  DEFAULT_MUSIC_VOLUME  = 1.0f;
constexpr float  DEFAULT_MIN_DISTANCE  = 1.0f;  ///< Minimum distance for 3D audio attenuation
constexpr float  DEFAULT_ATTENUATION   = 1.0f;  ///< Attenuation factor for 3D audio
constexpr float  DEFAULT_AUDIO_PITCH   = 1.0f;
constexpr float  MIN_VOLUME            = 0.0f;
constexpr float  MAX_VOLUME            = 1.0f;
}  // namespace AudioConstants

/**
 * @brief Configuration for fade effects
 *
 * @description
 * FadeConfig defines parameters for volume fade-in/fade-out transitions.
 * If duration is 0.0f, the fade is instant (no interpolation).
 */
struct FadeConfig
{
    float                      duration        = 0.0f;           ///< Fade duration in seconds (0.0f = instant)
    FadeCurve                  curve           = FadeCurve::Linear;  ///< Interpolation curve type
    bool                       allowInterrupt  = true;           ///< Whether this fade can be interrupted by another
    std::function<void()>      onComplete      = nullptr;        ///< Callback when fade completes (optional)

    /**
     * @brief Create an instant fade configuration (no interpolation)
     */
    static FadeConfig instant()
    {
        return FadeConfig{0.0f, FadeCurve::Linear, true, nullptr};
    }

    /**
     * @brief Create a linear fade configuration
     * @param fadeDuration Duration of the fade in seconds
     * @param interruptible Whether the fade can be interrupted
     */
    static FadeConfig linear(float fadeDuration, bool interruptible = true)
    {
        return FadeConfig{fadeDuration, FadeCurve::Linear, interruptible, nullptr};
    }
};

#endif  // AUDIOTYPES_H
