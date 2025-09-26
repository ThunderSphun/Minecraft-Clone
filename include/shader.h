#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <string>
#include <optional>
#include <filesystem>
#include <vector>
#include <memory>

namespace Minecraft::Assets {
	class Shader;
	class ShaderProgram;

	class Shader {
	public:
		Shader() = delete;
		Shader(GLenum shaderType);
		Shader(const Shader&) = delete;
		Shader(Shader&& other) noexcept;
		Shader& operator=(const Shader&) = delete;
		Shader& operator=(Shader&& other) noexcept;
		~Shader();

		void update();

		bool loadShaderSource(const std::string& source);
		bool loadShaderSource(const std::vector<std::string>& sources);

		static std::shared_ptr<Shader> parse(std::filesystem::path path);
		static std::shared_ptr<Shader> parse(std::filesystem::path path, GLenum shaderType);
		static std::shared_ptr<Shader> parse(const std::string& source, GLenum shaderType);

		friend class ShaderProgram;

	private:
		GLuint id = 0;

		std::optional<std::filesystem::path> path = {};
		std::filesystem::file_time_type lastTimeStamp = std::filesystem::file_time_type::min();

		std::vector<std::weak_ptr<ShaderProgram>> programs = {};
	};

	class ShaderProgram : public std::enable_shared_from_this<ShaderProgram> {
	public:
		ShaderProgram(const ShaderProgram&) = delete;
		ShaderProgram(ShaderProgram&& other) noexcept;
		ShaderProgram& operator=(const ShaderProgram&) = delete;
		ShaderProgram& operator=(ShaderProgram&& other) noexcept;
		~ShaderProgram();

		static std::shared_ptr<ShaderProgram> create();

		std::shared_ptr<ShaderProgram> attachShader(std::weak_ptr<Shader> shader);
		std::shared_ptr<ShaderProgram> detachShader(std::weak_ptr<Shader> shader);
		std::shared_ptr<ShaderProgram> bindAttribute(GLuint index, const std::string& name);
		std::shared_ptr<ShaderProgram> link();
		std::shared_ptr<ShaderProgram> use();

		void update();

		[[nodiscard]] GLint getUniform(const std::string& uniform);

		void setUniform(const std::string& uniform, bool value) { setUniform(getUniform(uniform), value); };
		void setUniform(const std::string& uniform, int value) { setUniform(getUniform(uniform), value); };
		void setUniform(const std::string& uniform, float value) { setUniform(getUniform(uniform), value); };
		void setUniform(const std::string& uniform, const glm::mat4& value) { setUniform(getUniform(uniform), value); };
		void setUniform(const std::string& uniform, const glm::mat3& value) { setUniform(getUniform(uniform), value); };
		void setUniform(const std::string& uniform, const glm::vec4& value) { setUniform(getUniform(uniform), value); };
		void setUniform(const std::string& uniform, const glm::vec3& value) { setUniform(getUniform(uniform), value); };
		void setUniform(const std::string& uniform, const glm::vec2& value) { setUniform(getUniform(uniform), value); };

		void setUniform(GLint uniform, bool value);
		void setUniform(GLint uniform, int value);
		void setUniform(GLint uniform, float value);
		void setUniform(GLint uniform, const glm::mat4& value);
		void setUniform(GLint uniform, const glm::mat3& value);
		void setUniform(GLint uniform, const glm::vec4& value);
		void setUniform(GLint uniform, const glm::vec3& value);
		void setUniform(GLint uniform, const glm::vec2& value);

		friend class Shader;

	private:
		ShaderProgram();

		GLuint id = 0;

		std::vector<std::shared_ptr<Shader>> shaders = {};
	};
}
