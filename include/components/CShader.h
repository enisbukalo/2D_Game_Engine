#ifndef CSHADER_H
#define CSHADER_H

#include <string>

namespace Components
{

/**
 * @brief Component for shader resources
 *
 * @description
 * CShader stores shader resource information for entities. It maintains the
 * paths to vertex and fragment shader files and acts as a reference for the
 * rendering system. The actual shader loading and compilation is handled by
 * the rendering system to avoid duplicate shader loads and improve performance.
 */
struct CShader
{
    CShader() = default;
    CShader(const std::string& vertexPath, const std::string& fragmentPath)
        : vertexShaderPath(vertexPath), fragmentShaderPath(fragmentPath)
    {
    }

    inline std::string getVertexShaderPath() const
    {
        return vertexShaderPath;
    }
    inline void setVertexShaderPath(const std::string& vertexPath)
    {
        vertexShaderPath = vertexPath;
    }
    inline std::string getFragmentShaderPath() const
    {
        return fragmentShaderPath;
    }
    inline void setFragmentShaderPath(const std::string& fragmentPath)
    {
        fragmentShaderPath = fragmentPath;
    }

    std::string vertexShaderPath;
    std::string fragmentShaderPath;
};

}  // namespace Components

#endif  // CSHADER_H
