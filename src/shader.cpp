#include "shader.h"

#include <fstream>
#include <iostream>

namespace Minecraft {
	GLuint createShader(const char* source, GLenum shaderType) {
		GLuint shaderID = glCreateShader(shaderType);

		if (shaderID == 0) {
			GLenum error = glGetError();
			if (error == GL_INVALID_ENUM)
				std::cerr << "shaderType is not valid, must be one of " <<
				"'GL_VERTEX_SHADER', 'GL_TESS_CONTROL_SHADER', 'GL_TESS_EVALUATION_SHADER', 'GL_GEOMETRY_SHADER', 'GL_FRAGMENT_SHADER', 'GL_COMPUTE_SHADER'"
				<< std::endl;
			else
				std::cerr << "unknown error from glCreateShader" << std::endl;

			return 0;
		}

		glShaderSource(shaderID, 1, &source, nullptr);

		glCompileShader(shaderID);
		{
			GLint error = 0;
			glGetShaderiv(shaderID, GL_COMPILE_STATUS, &error);
			if (error == GL_FALSE) {
				int length = 0;
				glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &length);
				char* errorMessage = new char[length + 1];
				memset(errorMessage, 0, length + 1);
				int writtenCount = 0;
				glGetShaderInfoLog(shaderID, length, &writtenCount, errorMessage);
				std::cerr << errorMessage << std::endl;
				errorMessage[length] = '\0';
				std::cerr << errorMessage << std::endl;
				delete[] errorMessage;

				glDeleteShader(shaderID);
				return 0;
			}
		}

		return shaderID;
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
			path = std::filesystem::path("assets") / path;
		else if (!path.has_parent_path())
			path = std::filesystem::path("assets") / "shaders" / path;

		if (!std::filesystem::exists(path)) {
			std::cout << "file " << path.filename() << " at " << path.parent_path() << " does not exist" << std::endl;
			return 0;
		}

		auto size = std::filesystem::file_size(path);
		char* str = new char[size];
		memset(str, 0, size);
		std::ifstream in(path);
		in.read(str, size);

		GLuint shaderID = createShader(str, shaderType);
		delete[] str;

		return shaderID;
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
