#ifndef SSCENE_H
#define SSCENE_H

#include <string>

/**
 * @brief System for managing game scenes and scene transitions
 *
 * @description
 * SScene is a singleton class responsible for managing game scenes,
 * handling scene transitions, and maintaining the scene stack. It ensures
 * that only one instance of the scene system exists throughout the application.
 */
class SScene
{
public:
    /**
     * @brief Gets the singleton instance of the SScene
     * @return Reference to the SScene instance
     */
    static SScene& instance()
    {
        static SScene instance;
        return instance;
    }

    /**
     * @brief Loads a scene from a file
     * @param scenePath Path to the scene file
     * @throws std::runtime_error if the scene cannot be loaded
     */
    void loadScene(const std::string& scenePath);

    /**
     * @brief Saves the current scene to its file
     * @throws std::runtime_error if no scene is loaded or if saving fails
     */
    void saveCurrentScene();

    /**
     * @brief Saves the current scene to a new file
     * @param scenePath Path where the scene should be saved
     * @throws std::runtime_error if saving fails
     */
    void saveScene(const std::string& scenePath);

    /**
     * @brief Gets the path of the currently loaded scene
     * @return The current scene path
     */
    const std::string& getCurrentScenePath() const;

    /**
     * @brief Clears the current scene
     */
    void clearScene();

    // Delete copy constructor and assignment operator
    SScene(const SScene&)            = delete;
    SScene& operator=(const SScene&) = delete;

private:
    // Private constructor to prevent direct instantiation
    SScene() = default;
    // Private destructor to prevent deletion through pointers
    ~SScene() = default;

    std::string m_currentScene = "";  ///< Path to the currently loaded scene
};

#endif
