#pragma once

#include <GL/glew.h>

#include <string_view>
#include <filesystem>

namespace Minecraft {
	GLuint createShader(const std::string_view& source, GLenum shaderType);
	GLuint createShader(std::filesystem::path path, GLenum shaderType);
	GLuint createShader(const std::filesystem::path& path);
}
