#pragma once

#include <string>
#include <filesystem>
#include <glm/glm.hpp>

namespace engine {

    class Shader {
    public:
        // Constructor to use path
        Shader(const std::filesystem::path& vertexPath,
            const std::filesystem::path& fragmentPath);

        ~Shader();

        void bind() const;
        unsigned int id() const;

        void setFloat(const std::string& name, float value) const;
        void setInt(const std::string& name, int value) const;
        void setBool(const std::string& name, bool value) const;
        void setVec3(const std::string& name, const glm::vec3& vec) const;
        void setMat4(const std::string& name, const glm::mat4& mat) const;

    private:
        unsigned int m_program = 0;

        // loadFile to use path
        static std::string loadFile(const std::filesystem::path& path);

        static unsigned int compile(unsigned int type,
            const std::string& src);
    };

} // namespace engine
