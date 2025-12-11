#include "SScene.h"
#include <spdlog/spdlog.h>
#include <filesystem>
#include <stdexcept>
#include "FileUtilities.h"
#include "SAudio.h"
#include "SystemLocator.h"
#include "SSerialization.h"

namespace Systems
{

void SScene::loadScene(const std::string& scenePath)
{
#if 0  // TODO: Update to use Registry for scene serialization
    // Check if file exists before attempting to load
    if (!std::filesystem::exists(scenePath))
    {
        throw std::runtime_error("Failed to load scene: File does not exist: " + scenePath);
    }

    try
    {
        ::Systems::SEntity::instance().clear();  // Clear existing entities
        ::Systems::SEntity::instance().loadFromFile(scenePath);

        // Load scene-level audio settings
        std::string                              json = ::Internal::FileUtilities::readFile(scenePath);
        Serialization::JsonParser                parser(json);
        Serialization::SSerialization::JsonValue root = Serialization::SSerialization::JsonValue::parse(parser);

        if (root.isObject() && root.hasKey("settings"))
        {
            const auto& settings = root["settings"];

            // Load and play scene music if specified
            if (settings.hasKey("music"))
            {
                std::string musicId = settings["music"].getString();
                if (!musicId.empty())
                {
                    auto& audioSystem = SystemLocator::audio();

                    // For now, assume music files are in a default location or use the ID as path
                    // In a production system, you'd use an asset manifest to map IDs to paths
                    std::string musicPath = "assets/music/" + musicId + ".ogg";

                    // Try to load and play the music
                    if (audioSystem.loadSound(musicId, musicPath, AudioType::Music))
                    {
                        audioSystem.playMusic(musicId, true);  // Loop by default
                        spdlog::info("Scene music '{}' loaded and playing", musicId);
                    }
                    else
                    {
                        spdlog::warn("Failed to load scene music: {}", musicPath);
                    }
                }
            }
        }

        m_currentScene = scenePath;  // Only set current scene if load succeeds
    }
    catch (const std::exception& e)
    {
        m_currentScene = "";  // Reset current scene on failure
        throw std::runtime_error("Failed to load scene: " + scenePath + "\nError: " + e.what());
    }
#endif
}

void SScene::saveCurrentScene()
{
#if 0  // TODO: Update to use Registry for scene serialization
    if (m_currentScene == "")
    {
        throw std::runtime_error("No scene is currently loaded");
    }

    try
    {
        ::Systems::SEntity::instance().saveToFile(m_currentScene);
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error("Failed to save scene: " + m_currentScene + "\nError: " + e.what());
    }
#endif
}

void SScene::saveScene(const std::string& scenePath)
{
#if 0  // TODO: Update to use Registry for scene serialization
    try
    {
        // Check if directory exists and is writable
        auto directory = std::filesystem::path(scenePath).parent_path();
        if (!directory.empty() && !std::filesystem::exists(directory))
        {
            throw std::runtime_error("Directory does not exist: " + directory.string());
        }

        ::Systems::SEntity::instance().saveToFile(scenePath);
        m_currentScene = scenePath;
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error("Failed to save scene: " + scenePath + "\nError: " + e.what());
    }
#endif
}

const std::string& SScene::getCurrentScenePath() const
{
    return m_currentScene;
}

void SScene::clearScene()
{
#if 0  // TODO: Update to use Registry
    // Stop any playing music
    SystemLocator::audio().stopMusic();

    ::Systems::SEntity::instance().clear();
    m_currentScene = "";
#endif
}

}  // namespace Systems
