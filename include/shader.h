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

	// TODO think about making 'GLuint shader' a shared_ptr instead, to get rid of needing shader to be a shared_ptr
	class Shader {
	public:
		Shader(GLenum shaderType);

		Shader() = delete;
		Shader(const Shader&) = delete;
		Shader(Shader&& other) noexcept;
		Shader& operator=(const Shader&) = delete;
		Shader& operator=(Shader&& other) noexcept;
		~Shader();

		bool update();

		bool loadShaderSource(const std::string& source);
		bool loadShaderSource(const std::vector<std::string>& sources);

		[[nodiscard]] static std::shared_ptr<Shader> parse(const std::filesystem::path& path);
		[[nodiscard]] static std::shared_ptr<Shader> parse(std::filesystem::path path, GLenum shaderType);
		[[nodiscard]] static std::shared_ptr<Shader> parse(const std::string& source, GLenum shaderType);

		using Program = ShaderProgram;

		friend class ShaderProgram;

	private:
		GLuint shader = 0;

		std::optional<std::filesystem::path> path = {};
		std::filesystem::file_time_type lastTimeStamp = std::filesystem::file_time_type::min();

		std::vector<std::weak_ptr<ShaderProgram>> programs = {};
	};

	// TODO think about making 'GLuint program' a shared_ptr instead, to get rid of needing shader to be a shared_ptr
	class ShaderProgram : public std::enable_shared_from_this<ShaderProgram> {
	public:
		ShaderProgram(const ShaderProgram&) = delete;
		ShaderProgram(ShaderProgram&& other) noexcept;
		ShaderProgram& operator=(const ShaderProgram&) = delete;
		ShaderProgram& operator=(ShaderProgram&& other) noexcept;
		~ShaderProgram();

		[[nodiscard]] static std::shared_ptr<ShaderProgram> create();

		std::shared_ptr<ShaderProgram> attachShader(std::weak_ptr<Shader> shader);
		std::shared_ptr<ShaderProgram> detachShader(std::weak_ptr<Shader> shader);
		std::shared_ptr<ShaderProgram> bindAttribute(GLuint index, const std::string& name);
		std::shared_ptr<ShaderProgram> link();
		std::shared_ptr<ShaderProgram> use();

		bool update();

		[[nodiscard]] GLint getUniform(const std::string& uniform) const;

		void setUniform(const std::string& uniform, bool val) { setUniform(getUniform(uniform), val); };
		void setUniform(const std::string& uniform, int val) { setUniform(getUniform(uniform), val); };
		void setUniform(const std::string& uniform, float val) { setUniform(getUniform(uniform), val); };
		void setUniform(const std::string& uniform, const glm::mat4& val) { setUniform(getUniform(uniform), val); };
		void setUniform(const std::string& uniform, const glm::mat3& val) { setUniform(getUniform(uniform), val); };
		void setUniform(const std::string& uniform, const glm::vec4& val) { setUniform(getUniform(uniform), val); };
		void setUniform(const std::string& uniform, const glm::vec3& val) { setUniform(getUniform(uniform), val); };
		void setUniform(const std::string& uniform, const glm::vec2& val) { setUniform(getUniform(uniform), val); };

		void setUniform(GLint uniform, bool val);
		void setUniform(GLint uniform, int val);
		void setUniform(GLint uniform, float val);
		void setUniform(GLint uniform, const glm::mat4& val);
		void setUniform(GLint uniform, const glm::mat3& val);
		void setUniform(GLint uniform, const glm::vec4& val);
		void setUniform(GLint uniform, const glm::vec3& val);
		void setUniform(GLint uniform, const glm::vec2& val);

		friend class Shader;

	private:
		ShaderProgram();

		GLuint program = 0;

		std::vector<std::shared_ptr<Shader>> shaders = {};
	};
}
