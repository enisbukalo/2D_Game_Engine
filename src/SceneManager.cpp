#include "SceneManager.h"
#include <filesystem>
#include <stdexcept>
#include "EntityManager.h"
#include "FileUtilities.h"

void SceneManager::loadScene(const std::string& scenePath)
{
    // Check if file exists before attempting to load
    if (!std::filesystem::exists(scenePath))
    {
        throw std::runtime_error("Failed to load scene: File does not exist: " + scenePath);
    }

    try
    {
        EntityManager::instance().clear();  // Clear existing entities
        EntityManager::instance().loadFromFile(scenePath);
        m_currentScene = scenePath;  // Only set current scene if load succeeds
    }
    catch (const std::exception& e)
    {
        m_currentScene = "";  // Reset current scene on failure
        throw std::runtime_error("Failed to load scene: " + scenePath + "\nError: " + e.what());
    }
}

void SceneManager::saveCurrentScene()
{
    if (m_currentScene == "")
    {
        throw std::runtime_error("No scene is currently loaded");
    }

    try
    {
        EntityManager::instance().saveToFile(m_currentScene);
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error("Failed to save scene: " + m_currentScene + "\nError: " + e.what());
    }
}

void SceneManager::saveScene(const std::string& scenePath)
{
    try
    {
        // Check if directory exists and is writable
        auto directory = std::filesystem::path(scenePath).parent_path();
        if (!directory.empty() && !std::filesystem::exists(directory))
        {
            throw std::runtime_error("Directory does not exist: " + directory.string());
        }

        EntityManager::instance().saveToFile(scenePath);
        m_currentScene = scenePath;
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error("Failed to save scene: " + scenePath + "\nError: " + e.what());
    }
}

const std::string& SceneManager::getCurrentScenePath() const
{
    return m_currentScene;
}

void SceneManager::clearScene()
{
    EntityManager::instance().clear();
    m_currentScene = "";
}
