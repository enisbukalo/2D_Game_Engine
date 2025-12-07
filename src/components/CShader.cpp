#include "CShader.h"
#include "SSerialization.h"

std::string CShader::getType() const
{
    return "Shader";
}

void CShader::serialize(Serialization::JsonBuilder& builder) const
{
    builder.beginObject();
    builder.addKey("cShader");
    builder.beginObject();
    builder.addKey("guid");
    builder.addString(getGuid());
    builder.addKey("vertexShaderPath");
    builder.addString(m_vertexShaderPath);
    builder.addKey("fragmentShaderPath");
    builder.addString(m_fragmentShaderPath);
    builder.endObject();
    builder.endObject();
}

void CShader::deserialize(const Serialization::SSerialization::JsonValue& value)
{
    const auto& shader = value["cShader"];
    if (shader.hasKey("guid"))
    {
        setGuid(shader["guid"].getString());
    }
    m_vertexShaderPath   = shader["vertexShaderPath"].getString();
    m_fragmentShaderPath = shader["fragmentShaderPath"].getString();
}

std::string CShader::getVertexShaderPath() const
{
    return m_vertexShaderPath;
}

void CShader::setVertexShaderPath(const std::string& vertexPath)
{
    m_vertexShaderPath = vertexPath;
}

std::string CShader::getFragmentShaderPath() const
{
    return m_fragmentShaderPath;
}

void CShader::setFragmentShaderPath(const std::string& fragmentPath)
{
    m_fragmentShaderPath = fragmentPath;
}
