#include "CShader.h"

namespace Components
{

std::string CShader::getVertexShaderPath() const
{
    return vertexShaderPath;
}

void CShader::setVertexShaderPath(const std::string& vertexPath)
{
    vertexShaderPath = vertexPath;
}

std::string CShader::getFragmentShaderPath() const
{
    return fragmentShaderPath;
}

void CShader::setFragmentShaderPath(const std::string& fragmentPath)
{
    fragmentShaderPath = fragmentPath;
}

}  // namespace Components
