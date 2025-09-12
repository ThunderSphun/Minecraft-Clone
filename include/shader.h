#pragma once

#include <GL/glew.h>

#include <filesystem>

namespace Minecraft {
	GLuint createShader(const char* source, GLenum shaderType);
	GLuint createShader(std::filesystem::path path, GLenum shaderType);
	GLuint createShader(const std::filesystem::path& path);
}
