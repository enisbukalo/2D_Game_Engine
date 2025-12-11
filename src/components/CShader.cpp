#include "CShader.h"
#include "SSerialization.h"

namespace Components
{

std::string CShader::getType() const
{
    return "Shader";
}

void CShader::serialize(Serialization::JsonBuilder& builder) const
{
    builder.beginObject();
    builder.addKey("cShader");
    builder.beginObject();
    builder.addKey("vertexShaderPath");
    builder.addString(vertexShaderPath);
    builder.addKey("fragmentShaderPath");
    builder.addString(fragmentShaderPath);
    builder.endObject();
    builder.endObject();
}

void CShader::deserialize(const Serialization::SSerialization::JsonValue& value)
{
    const auto& shader = value["cShader"];
    vertexShaderPath   = shader["vertexShaderPath"].getString();
    fragmentShaderPath = shader["fragmentShaderPath"].getString();
}

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
