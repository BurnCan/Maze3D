#pragma once

#include <string>
#include <glm/glm.hpp>

namespace engine {

class Shader {
public:
    Shader(const std::string& vertexPath,
           const std::string& fragmentPath);

    ~Shader();

    void bind() const;
    unsigned int id() const;

    // uniforms
    void setMat4(const std::string& name, const glm::mat4& mat) const;
    void setVec3(const std::string& name, const glm::vec3& vec) const;

private:
    unsigned int m_program = 0;

    static std::string loadFile(const std::string& path);
    static unsigned int compile(unsigned int type,
                                const std::string& src);
};

} // namespace engine
