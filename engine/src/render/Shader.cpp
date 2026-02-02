#include "engine/render/Shader.h"

#include <fstream>
#include <sstream>
#include <iostream>

#include <glad/glad.h>

namespace engine {

static void checkLink(unsigned int program)
{
    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char info[1024];
        glGetProgramInfoLog(program, 1024, nullptr, info);
        throw std::runtime_error(info);
    }
}

static void checkCompile(unsigned int shader)
{
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info[1024];
        glGetShaderInfoLog(shader, 1024, nullptr, info);
        throw std::runtime_error(info);
    }
}

Shader::Shader(const std::string& vertPath,
               const std::string& fragPath)
{
    std::string vertSrc = loadFile(vertPath);
    std::string fragSrc = loadFile(fragPath);

    unsigned int vs = compile(GL_VERTEX_SHADER, vertSrc);
    unsigned int fs = compile(GL_FRAGMENT_SHADER, fragSrc);

    m_program = glCreateProgram();
    glAttachShader(m_program, vs);
    glAttachShader(m_program, fs);
    glLinkProgram(m_program);

    checkLink(m_program);

    glDeleteShader(vs);
    glDeleteShader(fs);
}

Shader::~Shader()
{
    if (m_program)
        glDeleteProgram(m_program);
}

unsigned int Shader::id() const
{
    return m_program;
}

void Shader::bind() const
{
    glUseProgram(m_program);
}

void Shader::setMat4(const std::string& name,
                     const glm::mat4& mat) const
{
    int loc = glGetUniformLocation(m_program, name.c_str());
    glUniformMatrix4fv(loc, 1, GL_FALSE, &mat[0][0]);
}

void Shader::setVec3(const std::string& name,
                     const glm::vec3& vec) const
{
    int loc = glGetUniformLocation(m_program, name.c_str());
    glUniform3fv(loc, 1, &vec[0]);
}

std::string Shader::loadFile(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open())
        throw std::runtime_error("Failed to open shader: " + path);

    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

unsigned int Shader::compile(unsigned int type,
                             const std::string& src)
{
    unsigned int shader = glCreateShader(type);
    const char* cstr = src.c_str();
    glShaderSource(shader, 1, &cstr, nullptr);
    glCompileShader(shader);

    checkCompile(shader);
    return shader;
}

} // namespace engine
