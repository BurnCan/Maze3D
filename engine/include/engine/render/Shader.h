#pragma once

#include <string>
#include <filesystem> // 1. Must include this
#include <glm/glm.hpp>

namespace engine {

    class Shader {
    public:
        // 2. Update constructor to use path
        Shader(const std::filesystem::path& vertexPath,
            const std::filesystem::path& fragmentPath);

        ~Shader();

        void bind() const;
        unsigned int id() const;

        void setMat4(const std::string& name, const glm::mat4& mat) const;
        void setVec3(const std::string& name, const glm::vec3& vec) const;

    private:
        unsigned int m_program = 0;

        // 3. Update loadFile to use path
        static std::string loadFile(const std::filesystem::path& path);

        static unsigned int compile(unsigned int type,
            const std::string& src);
    };

} // namespace engine
