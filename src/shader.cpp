#include "shader.h"

#include <iostream>
#include <cstring>
#include <sys/stat.h>

Shader::Shader(const std::string& path, GLenum shaderType) {
	{
		std::string roughPath = path;
		if (!parsePath(roughPath, shaderType))
			throw std::runtime_error(roughPath);
		this->path = roughPath;
	}

	std::string shaderString = readEntireFile();

	if (!compile(shaderString.c_str(), shaderType))
		throw std::runtime_error(lastError);
}

bool Shader::shouldUpdate() const {
	if (!path.has_value()) {
		return false;
	}

	struct stat result{};

	if (stat(path->c_str(), &result) == -1) {
		std::cerr << "filepath somehow invalid" << std::endl << "ignoring update" << std::endl;
		return false;
	}

	bool retVal = false;
	time_t fileChangedTimeStamp = result.st_mtime;
	if (fileChangedTimeStamp > lastTimeStamp)
		retVal = true;
	lastTimeStamp = fileChangedTimeStamp;

	return retVal;
}

bool Shader::compile(const char* source, GLenum shaderType) {
	if (id != -1)
		glDeleteShader(id);

	switch (shaderType) {
		case GL_FRAGMENT_SHADER:
			id = glCreateShader(GL_FRAGMENT_SHADER);
			break;
		case GL_VERTEX_SHADER:
			id = glCreateShader(GL_VERTEX_SHADER);
			break;
		default:
			lastError = "shader type not supported";
			return false;
	}
	if (glGetError() == GL_INVALID_ENUM) {
		lastError = "passed in wrong shader type";
		id = -1;
		return false;
	}
	if (id == 0) {
		lastError = "internal OpenGL error";
		id = -1;
		return false;
	}

	glShaderSource(id, 1, &source, nullptr);
	{
		GLint error = 0;
		glGetShaderiv(id, GL_SHADER_SOURCE_LENGTH, &error);
		if (error == 0) {
			lastError = "could not upload shader source to OpenGL";
			glDeleteShader(id);
			id = -1;
			return false;
		}
	}

	glCompileShader(id);
	{
		GLint error = 0;
		glGetShaderiv(id, GL_COMPILE_STATUS, &error);
		if (error == GL_FALSE) {
			int length = 0;
			glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
			char* errorMessage = new char[length + 1];
			memset(errorMessage, 0, length + 1);
			int writtenCount = 0;
			glGetShaderInfoLog(id, length, &writtenCount, errorMessage);
			lastError = std::string(errorMessage);
			delete[] errorMessage;

			glDeleteShader(id);
			return false;
		}
	}

	return true;
}

bool Shader::parsePath(std::string& pathToParse, GLenum shaderType) {
	if (pathToParse.find('\\') == std::string::npos && pathToParse.find('/') == std::string::npos)
		pathToParse = "assets/shaders/" + pathToParse;
	if (pathToParse.find('.') == std::string::npos) {
		switch (shaderType) {
			case GL_FRAGMENT_SHADER:
				pathToParse += ".frag";
				break;
			case GL_VERTEX_SHADER:
				pathToParse += ".vert";
				break;
			default:
				pathToParse = "";
				lastError = "shader type not supported";
				return false;
		}
	}

	return true;
}

std::string Shader::readEntireFile() {
	struct stat result{};

	if (stat(path->c_str(), &result) == -1) {
		lastError = "file path '" + path.value() + "' invalid";
		throw std::runtime_error(lastError);
	}

	std::ifstream file(path->c_str());

	if (!file) {
		lastError = "file path '" + path.value() + "' invalid";
		throw std::runtime_error(lastError);
	}

	std::string shaderString((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	if (shaderString.empty()) {
		lastError = "file wasn't loaded";
		throw std::runtime_error(lastError);
	}

	lastTimeStamp = result.st_mtime;

	return shaderString;
}
