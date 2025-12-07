#ifndef CSHADER_H
#define CSHADER_H

#include <string>
#include "Component.h"

/**
 * @brief Component for shader resources
 *
 * @description
 * CShader stores shader resource information for entities. It maintains the
 * paths to vertex and fragment shader files and acts as a reference for the
 * rendering system. The actual shader loading and compilation is handled by
 * the rendering system to avoid duplicate shader loads and improve performance.
 */
struct CShader : public Component
{
public:
    /** @brief Default constructor */
    CShader() = default;

    /**
     * @brief Constructs a shader component with file paths
     * @param vertexPath Path to the vertex shader file (optional)
     * @param fragmentPath Path to the fragment shader file (optional)
     */
    CShader(const std::string& vertexPath, const std::string& fragmentPath)
        : m_vertexShaderPath(vertexPath), m_fragmentShaderPath(fragmentPath)
    {
    }

    /**
     * @brief Gets the type identifier for this component
     * @return String "Shader"
     */
    std::string getType() const override;

    /**
     * @brief Serializes the shader data
     */
    void serialize(Serialization::JsonBuilder& builder) const override;

    /**
     * @brief Deserializes shader data from JSON value
     */
    void deserialize(const Serialization::SSerialization::JsonValue& value) override;

    /**
     * @brief Gets the vertex shader file path
     * @return Path to vertex shader file
     */
    std::string getVertexShaderPath() const;

    /**
     * @brief Sets the vertex shader file path
     * @param vertexPath New path to vertex shader file
     */
    void setVertexShaderPath(const std::string& vertexPath);

    /**
     * @brief Gets the fragment shader file path
     * @return Path to fragment shader file
     */
    std::string getFragmentShaderPath() const;

    /**
     * @brief Sets the fragment shader file path
     * @param fragmentPath New path to fragment shader file
     */
    void setFragmentShaderPath(const std::string& fragmentPath);

private:
    std::string m_vertexShaderPath;    ///< Path to the vertex shader file
    std::string m_fragmentShaderPath;  ///< Path to the fragment shader file
};

#endif  // CSHADER_H
