#pragma once

#include <GL/glew.h>

#include <filesystem>
#include <string_view>

namespace Minecraft {
	GLuint createShader(const std::string_view& source, GLenum shaderType);
	GLuint createShader(std::filesystem::path path, GLenum shaderType);
	GLuint createShader(const std::filesystem::path& path);
}
