#include "shader.h"

#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <algorithm>

namespace Minecraft::Assets {
	ShaderProgram::ShaderProgram() : id(glCreateProgram()) {
		if (id == 0)
			std::cerr << "unknown error from glCreateProgram" << std::endl;
	}

	ShaderProgram::ShaderProgram(ShaderProgram&& other) noexcept {
		if (id != 0)
			glDeleteProgram(id);

		this->id = other.id;
		this->shaders = other.shaders;

		other.id = 0;
		other.shaders = {};
	}

	ShaderProgram& ShaderProgram::operator=(ShaderProgram&& other) noexcept {
		if (this != &other) {
			if (id != 0)
				glDeleteProgram(id);

			this->id = other.id;
			this->shaders = other.shaders;

			other.id = 0;
			other.shaders = {};
		}
		return *this;
	}

	ShaderProgram::~ShaderProgram() {
		if (id != 0)
			glDeleteProgram(id);
	}

	std::shared_ptr<ShaderProgram> ShaderProgram::create() {
		return std::shared_ptr<ShaderProgram>(new ShaderProgram());
	}

	std::shared_ptr<ShaderProgram> ShaderProgram::attachShader(std::weak_ptr<Shader> _shader) {
		if (auto shader = _shader.lock()) {
			GLint shaderCount = 0;
			glGetProgramiv(id, GL_ATTACHED_SHADERS, &shaderCount);
			std::vector<GLuint> attachedShaders(shaderCount);
			glGetAttachedShaders(id, shaderCount, &shaderCount, attachedShaders.data());
			if (std::none_of(attachedShaders.begin(), attachedShaders.end(), [shader](GLuint i) { return i == shader->id; })) {
				glAttachShader(id, shader->id);
				{
					GLenum error = glGetError();
					if (error == GL_INVALID_VALUE) {
						if (!glIsProgram(id))
							std::cerr << "provided program '" << id << "' isn't known to opengl" << std::endl;
						if (!glIsShader(shader->id))
							std::cerr << "provided shader '" << shader->id << "' isn't known to opengl" << std::endl;

						return shared_from_this();
					} else if (error == GL_INVALID_OPERATION) {
						if (!glIsProgram(id))
							std::cerr << "provided program '" << id << "' isn't a program" << std::endl;
						if (!glIsShader(shader->id))
							std::cerr << "provided shader '" << shader->id << "' isn't a shader" << std::endl;
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
			glGetProgramiv(id, GL_ATTACHED_SHADERS, &shaderCount);
			std::vector<GLuint> attachedShaders(shaderCount);
			glGetAttachedShaders(id, shaderCount, &shaderCount, attachedShaders.data());
			if (std::any_of(attachedShaders.begin(), attachedShaders.end(), [shader](GLuint i) { return i == shader->id; })) {
				glDetachShader(id, shader->id);
				{
					GLenum error = glGetError();
					if (error == GL_INVALID_VALUE) {
						if (!glIsProgram(id))
							std::cerr << "provided program '" << id << "' isn't known to opengl" << std::endl;
						if (!glIsShader(shader->id))
							std::cerr << "provided shader '" << shader->id << "' isn't known to opengl" << std::endl;

						return shared_from_this();
					} else if (error == GL_INVALID_OPERATION) {
						if (!glIsProgram(id))
							std::cerr << "provided program '" << id << "' isn't a program" << std::endl;
						if (!glIsShader(shader->id))
							std::cerr << "provided shader '" << shader->id << "' isn't a shader" << std::endl;
						// error code is technicly possible when the shader is not attached to the program,
						// but we explicitly check for it not being the case

						return shared_from_this();
					}
				}

				std::erase_if(shader->programs, [this](const std::weak_ptr<ShaderProgram>& _program) {
					if (auto program = _program.lock())
						return program->id == id;
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
		if (index >= GL_MAX_VERTEX_ATTRIBS) {
			std::cerr << "index '" << index << "' is out of range [0, " << GL_MAX_VERTEX_ATTRIBS << ")" << std::endl;
			return shared_from_this();
		}

		glBindAttribLocation(id, index, name.c_str());

		return shared_from_this();
	}

	std::shared_ptr<ShaderProgram> ShaderProgram::link() {
		glLinkProgram(id);
		{
			GLint isLinked = 0;
			glGetProgramiv(id, GL_LINK_STATUS, &isLinked);

			if (isLinked == GL_FALSE) {
				int length = 0;
				glGetProgramiv(id, GL_INFO_LOG_LENGTH, &length);
				std::string errorMessage(length, '\0');
				int writtenCount = 0;
				glGetProgramInfoLog(id, length, &writtenCount, errorMessage.data());
				std::cerr << "shader program linked with error:\n" << errorMessage << std::endl;
			}
		}

		return shared_from_this();
	}

	std::shared_ptr<ShaderProgram> ShaderProgram::use() {
		GLint isLinked = 0;
		glGetProgramiv(id, GL_LINK_STATUS, &isLinked);

		if (isLinked == GL_TRUE)
			glUseProgram(id);
		else
			std::cerr << "Attempted to use unlinked program" << std::endl;

		return shared_from_this();
	}

	void ShaderProgram::update() {
		for (auto& shader : shaders)
			shader->update();
	}

	GLint ShaderProgram::getUniform(const std::string& uniform) {
		return glGetUniformLocation(id, uniform.c_str());
}

	void ShaderProgram::setUniform(GLint uniform, bool val) { glUniform1i(uniform, val ? GL_TRUE : GL_FALSE); }
	void ShaderProgram::setUniform(GLint uniform, int val) { glUniform1i(uniform, val); }
	void ShaderProgram::setUniform(GLint uniform, float val) { glUniform1f(uniform, val); }
	void ShaderProgram::setUniform(GLint uniform, const glm::mat4& val) { glUniformMatrix4fv(uniform, 1, GL_FALSE, glm::value_ptr(val)); }
	void ShaderProgram::setUniform(GLint uniform, const glm::mat3& val) { glUniformMatrix3fv(uniform, 1, GL_FALSE, glm::value_ptr(val)); }
	void ShaderProgram::setUniform(GLint uniform, const glm::vec4& val) { glUniform4f(uniform, val.x, val.y, val.z, val.w); }
	void ShaderProgram::setUniform(GLint uniform, const glm::vec3& val) { glUniform3f(uniform, val.x, val.y, val.z); }
	void ShaderProgram::setUniform(GLint uniform, const glm::vec2& val) { glUniform2f(uniform, val.x, val.y); }
}
