#pragma once

#include <GL/glew.h>

#include <string>
#include <fstream>
#include <optional>

class Shader {
public:
	Shader(const std::string& path, GLenum shaderType);
	Shader() = default;

	std::string lastError;

private:
	friend class ShaderProgram;

	bool compile(const char* source, GLenum shaderType);
	[[nodiscard]] bool shouldUpdate() const;

	bool parsePath(std::string& pathToParse, GLenum shaderType);
	std::string readEntireFile();

	std::optional<std::string> path;
	GLuint id = -1;
	mutable time_t lastTimeStamp = 0;
};