#ifndef AUDIOTYPES_H
#define AUDIOTYPES_H

#include <cstddef>
#include <cstdint>

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
    SFX,
    Music
};

/**
 * @brief Audio system constants
 */
namespace AudioConstants
{
constexpr size_t DEFAULT_SFX_POOL_SIZE = 32;
constexpr float  DEFAULT_MASTER_VOLUME = 1.0f;
constexpr float  DEFAULT_SFX_VOLUME    = 1.0f;
constexpr float  DEFAULT_MUSIC_VOLUME  = 1.0f;
constexpr float  MIN_VOLUME            = 0.0f;
constexpr float  MAX_VOLUME            = 1.0f;
}  // namespace AudioConstants

#endif  // AUDIOTYPES_H
