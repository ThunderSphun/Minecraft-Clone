/*#include "shaderProgram.h"

#include <glm/gtc/type_ptr.hpp>

#include <iostream>

ShaderProgram::ShaderProgram() {
	id = glCreateProgram();
}

ShaderProgram::~ShaderProgram() {
	if (id != -1) {
		glDeleteProgram(id);
	}
}

ShaderProgram* ShaderProgram::attachShader(const Shader& shader) {
	shaders.push_back(shader);
	glAttachShader(id, shader.id);

	switch (glGetError()) {
		case GL_INVALID_ENUM:
			std::cerr << "enum isn't valid" << std::endl;
			break;
		case GL_INVALID_VALUE:
			std::cerr << "shader id isn't valid" << std::endl;
			break;
		case GL_INVALID_OPERATION:
			if (glIsShader(shader.id))
				std::cerr << "shader id isn't a shader" << std::endl;
			else
				std::cerr << "unknown invalid operation" << std::endl;
			break;
	}

	return this;
}

ShaderProgram* ShaderProgram::bindAttribute(int index, const char* name) {
	glBindAttribLocation(id, index, name);

	return this;
}

ShaderProgram* ShaderProgram::linkProgram() {
	glLinkProgram(id);

	{
		GLint error = 0;
		glGetProgramiv(id, GL_LINK_STATUS, &error);

		if (error == GL_FALSE) {
			int length = 0;
			glGetProgramiv(id, GL_INFO_LOG_LENGTH, &length);
			char* errorMessage = new char[length + 1];
			memset(errorMessage, 0, length + 1);
			int writtenCount = 0;
			glGetProgramInfoLog(id, length, &writtenCount, errorMessage);
			std::cerr << errorMessage << std::endl;
			delete[] errorMessage;
		}
	}

	return this;
}

ShaderProgram* ShaderProgram::useProgram() {
	glUseProgram(id);

	return this;
}

void ShaderProgram::update() {
	GLint currentProgramId;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgramId);

	bool shouldReLink = false;

	for (auto& shader : shaders) {
		if (!shader.shouldUpdate())
			continue;

		GLint shaderType = 0;
		GLuint oldID = shader.id;
		glGetShaderiv(oldID, GL_SHADER_TYPE, &shaderType);

		if (shader.compile(shader.readEntireFile().c_str(), shaderType)) {

			glDetachShader(id, oldID);
			glDeleteShader(oldID);
			glAttachShader(id, shader.id);

			shouldReLink = true;

		} else {
			std::cerr << shader.lastError << std::endl;
		}
	}

	if (shouldReLink)
		linkProgram();

	if (currentProgramId == id)
		useProgram();
}

GLint ShaderProgram::getUniform(const std::string& uniform) const {
	return glGetUniformLocation(id, uniform.c_str());
}

void ShaderProgram::setUniform(const std::string& uniform, const glm::mat4& value) const {
	setUniform(getUniform(uniform), value);
}

void ShaderProgram::setUniform(const std::string& uniform, const glm::mat3& value) const {
	setUniform(getUniform(uniform), value);
}

void ShaderProgram::setUniform(const std::string& uniform, const glm::vec4& value) const {
	setUniform(getUniform(uniform), value);
}

void ShaderProgram::setUniform(const std::string& uniform, const glm::vec3& value) const {
	setUniform(getUniform(uniform), value);
}

void ShaderProgram::setUniform(const std::string& uniform, const glm::vec2& value) const {
	setUniform(getUniform(uniform), value);
}

void ShaderProgram::setUniform(const std::string& uniform, bool value) const {
	setUniform(getUniform(uniform), value);
}

void ShaderProgram::setUniform(const std::string& uniform, int value) const {
	setUniform(getUniform(uniform), value);
}

void ShaderProgram::setUniform(const std::string& uniform, float value) const {
	setUniform(getUniform(uniform), value);
}

void ShaderProgram::setUniform(GLint uniform, bool value) const {
	glUniform1i(uniform, value ? GL_TRUE : GL_FALSE);
}

void ShaderProgram::setUniform(GLint uniform, int value) const {
	glUniform1i(uniform, value);
}

void ShaderProgram::setUniform(GLint uniform, float value) const {
	glUniform1f(uniform, value);
}

void ShaderProgram::setUniform(GLint uniform, const glm::mat4& value) const {
	glUniformMatrix4fv(uniform, 1, GL_FALSE, glm::value_ptr(value));
}

void ShaderProgram::setUniform(GLint uniform, const glm::mat3& value) const {
	glUniformMatrix3fv(uniform, 1, GL_FALSE, glm::value_ptr(value));
}

void ShaderProgram::setUniform(GLint uniform, const glm::vec4& value) const {
	glUniform4fv(uniform, 1, glm::value_ptr(value));
}

void ShaderProgram::setUniform(GLint uniform, const glm::vec3& value) const {
	glUniform3fv(uniform, 1, glm::value_ptr(value));
}

void ShaderProgram::setUniform(GLint uniform, const glm::vec2& value) const {
	glUniform2fv(uniform, 1, glm::value_ptr(value));
}

*/