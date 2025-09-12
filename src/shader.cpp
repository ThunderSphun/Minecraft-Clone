#include "shader.h"

#include <string>
#include <fstream>
#include <iostream>
#include <format>

namespace Minecraft {
	GLuint createShader(const std::string_view& source, GLenum shaderType) {
		return 0;
	}

	GLuint createShader(std::filesystem::path path, GLenum shaderType) {
		if (path.empty()) return 0;
		if (!path.has_extension()) {
			switch (shaderType) {
				case GL_VERTEX_SHADER: path += ".vert"; break;
				case GL_TESS_CONTROL_SHADER: path += ".tesc"; break;
				case GL_TESS_EVALUATION_SHADER: path += ".tese"; break;
				case GL_GEOMETRY_SHADER: path += ".geom"; break;
				case GL_FRAGMENT_SHADER: path += ".frag"; break;
				case GL_COMPUTE_SHADER: path += ".comp"; break;
				default: return 0;
			}
		}
		if (path.has_parent_path() && path.parent_path() == "shaders")
			path = std::filesystem::path(ASSETS_ROOT) / path;
		else if (!path.has_parent_path())
			path = std::filesystem::path(ASSETS_ROOT) / "shaders" / path;

		if (!std::filesystem::exists(path)) {
			std::cout << "file " << path.filename() << " at " << path.parent_path() << " does not exist" << std::endl;
			return 0;
		}

		auto size = std::filesystem::file_size(path);
		std::string str(size, '\0');
		std::ifstream in(path);
		in.read(&str[0], size);

		return createShader(std::string_view(str), shaderType);
	}

	GLuint createShader(const std::filesystem::path& path) {
		std::filesystem::path extension = path.extension();
		if (extension == ".vert") return createShader(path, GL_VERTEX_SHADER);
		if (extension == ".tesc") return createShader(path, GL_TESS_CONTROL_SHADER);
		if (extension == ".tese") return createShader(path, GL_TESS_EVALUATION_SHADER);
		if (extension == ".geom") return createShader(path, GL_GEOMETRY_SHADER);
		if (extension == ".frag") return createShader(path, GL_FRAGMENT_SHADER);
		if (extension == ".comp") return createShader(path, GL_COMPUTE_SHADER);

		std::cerr << "could not refer shader type from path " << path << std::endl;
		return 0;
	}
}
