#ifndef AUDIOTYPES_H
#define AUDIOTYPES_H

#include <cstdint>
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

#endif  // AUDIOTYPES_H
