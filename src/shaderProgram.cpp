#include "shader.h"

#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <algorithm>

namespace Minecraft::Assets {
	ShaderProgram::ShaderProgram() : program(glCreateProgram()) {
		if (program == 0)
			std::cerr << "unknown error from glCreateProgram" << std::endl;
	}

	ShaderProgram::ShaderProgram(ShaderProgram&& other) noexcept {
		if (program != 0)
			glDeleteProgram(program);

		this->program = other.program;
		this->shaders = other.shaders;

		other.program = 0;
		other.shaders = {};
	}

	ShaderProgram& ShaderProgram::operator=(ShaderProgram&& other) noexcept {
		if (this != &other) {
			if (program != 0)
				glDeleteProgram(program);

			this->program = other.program;
			this->shaders = other.shaders;

			other.program = 0;
			other.shaders = {};
		}
		return *this;
	}

	ShaderProgram::~ShaderProgram() {
		if (program != 0)
			glDeleteProgram(program);
	}

	std::shared_ptr<ShaderProgram> ShaderProgram::create() {
		return std::shared_ptr<ShaderProgram>(new ShaderProgram());
	}

	std::shared_ptr<ShaderProgram> ShaderProgram::attachShader(std::weak_ptr<Shader> _shader) {
		if (auto shader = _shader.lock()) {
			GLint shaderCount = 0;
			glGetProgramiv(program, GL_ATTACHED_SHADERS, &shaderCount);
			std::vector<GLuint> attachedShaders(shaderCount);
			glGetAttachedShaders(program, shaderCount, &shaderCount, attachedShaders.data());
			if (std::none_of(attachedShaders.begin(), attachedShaders.end(), [shader](GLuint i) { return i == shader->shader; })) {
				glAttachShader(program, shader->shader);
				{
					GLenum error = glGetError();
					if (error == GL_INVALID_VALUE) {
						if (!glIsProgram(program))
							std::cerr << "provided program '" << program << "' isn't known to opengl" << std::endl;
						if (!glIsShader(shader->shader))
							std::cerr << "provided shader '" << shader->shader << "' isn't known to opengl" << std::endl;

						return shared_from_this();
					} else if (error == GL_INVALID_OPERATION) {
						if (!glIsProgram(program))
							std::cerr << "provided program '" << program << "' isn't a program" << std::endl;
						if (!glIsShader(shader->shader))
							std::cerr << "provided shader '" << shader->shader << "' isn't a shader" << std::endl;
						// error code is technicly possible when the shader is already attached to the program,
						// but we explicitly check for it not being the case

						return shared_from_this();
					}
				}
				shader->programs.push_back(weak_from_this());
				shaders.push_back(shader);
			}
		}

		return shared_from_this();
	}

	std::shared_ptr<ShaderProgram> ShaderProgram::detachShader(std::weak_ptr<Shader> _shader) {
		if (auto shader = _shader.lock()) {
			GLint shaderCount = 0;
			glGetProgramiv(program, GL_ATTACHED_SHADERS, &shaderCount);
			std::vector<GLuint> attachedShaders(shaderCount);
			glGetAttachedShaders(program, shaderCount, &shaderCount, attachedShaders.data());
			if (std::any_of(attachedShaders.begin(), attachedShaders.end(), [shader](GLuint i) { return i == shader->shader; })) {
				glDetachShader(program, shader->shader);
				{
					GLenum error = glGetError();
					if (error == GL_INVALID_VALUE) {
						if (!glIsProgram(program))
							std::cerr << "provided program '" << program << "' isn't known to opengl" << std::endl;
						if (!glIsShader(shader->shader))
							std::cerr << "provided shader '" << shader->shader << "' isn't known to opengl" << std::endl;

						return shared_from_this();
					} else if (error == GL_INVALID_OPERATION) {
						if (!glIsProgram(program))
							std::cerr << "provided program '" << program << "' isn't a program" << std::endl;
						if (!glIsShader(shader->shader))
							std::cerr << "provided shader '" << shader->shader << "' isn't a shader" << std::endl;
						// error code is technicly possible when the shader is not attached to the program,
						// but we explicitly check for it not being the case

						return shared_from_this();
					}
				}

				std::erase_if(shader->programs, [this](const std::weak_ptr<ShaderProgram>& _program) {
					if (auto program = _program.lock())
						return program->program == this->program;
					return false;
				});
				std::erase(shaders, shader);
			}
		}

		return shared_from_this();
	}

	std::shared_ptr<ShaderProgram> ShaderProgram::bindAttribute(GLuint index, const std::string& name) {
		if (name.starts_with("gl_")) {
			std::cerr << "attribute '" << name << "' starts with reserved prefix 'gl_'" << std::endl;
			return shared_from_this();
		}
		{
			int maxVertexCount = 0;
			glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxVertexCount);
			if (index >= maxVertexCount) {
				std::cerr << "index '" << index << "' is out of range [0, " << maxVertexCount << ")" << std::endl;
				return shared_from_this();
			}
		}

		// errors should all be handled already
		glBindAttribLocation(program, index, name.c_str());

		return shared_from_this();
	}

	std::shared_ptr<ShaderProgram> ShaderProgram::link() {
		glLinkProgram(program);
		{
			GLint isLinked = 0;
			glGetProgramiv(program, GL_LINK_STATUS, &isLinked);

			if (isLinked == GL_FALSE) {
				int length = 0;
				glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
				std::string errorMessage(length, '\0');
				int writtenCount = 0;
				glGetProgramInfoLog(program, length, &writtenCount, errorMessage.data());
				std::cerr << "shader program linked with error:\n" << errorMessage << std::endl;
			}
		}

		return shared_from_this();
	}

	std::shared_ptr<ShaderProgram> ShaderProgram::use() {
		GLint isLinked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, &isLinked);

		if (isLinked == GL_TRUE)
			glUseProgram(program);
		else
			std::cerr << "Attempted to use unlinked program" << std::endl;

		return shared_from_this();
	}

	bool ShaderProgram::update() {
		bool hasUpdated = false;

		for (auto& shader : shaders)
			hasUpdated |= shader->update();

		return hasUpdated;
	}

	GLint ShaderProgram::getUniform(const std::string& uniform) const {
		return glGetUniformLocation(program, uniform.c_str());
	}

	void ShaderProgram::setUniform(GLint uniform, bool value) { glUniform1i(uniform, value ? GL_TRUE : GL_FALSE); }
	void ShaderProgram::setUniform(GLint uniform, int value) { glUniform1i(uniform, value); }
	void ShaderProgram::setUniform(GLint uniform, float value) { glUniform1f(uniform, value); }
	void ShaderProgram::setUniform(GLint uniform, const glm::mat4& value) { glUniformMatrix4fv(uniform, 1, GL_FALSE, glm::value_ptr(value)); }
	void ShaderProgram::setUniform(GLint uniform, const glm::mat3& value) { glUniformMatrix3fv(uniform, 1, GL_FALSE, glm::value_ptr(value)); }
	void ShaderProgram::setUniform(GLint uniform, const glm::vec4& value) { glUniform4fv(uniform, 1, glm::value_ptr(value)); }
	void ShaderProgram::setUniform(GLint uniform, const glm::vec3& value) { glUniform3fv(uniform, 1, glm::value_ptr(value)); }
	void ShaderProgram::setUniform(GLint uniform, const glm::vec2& value) { glUniform2fv(uniform, 1, glm::value_ptr(value)); }
}
