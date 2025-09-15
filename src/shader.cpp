#include "shader.h"

#include <fstream>
#include <iostream>
#include <array>

namespace Assets {
	Shader::Program::Program() : id(glCreateProgram()), fragmentShaderIndex(-1), vertexShaderIndex(-1) {
	}

	Shader::Program::~Program() {
		if (vertexShaderIndex != -1)
			shaderStorage.shaders[vertexShaderIndex].programIndex = -1;
		if (fragmentShaderIndex != -1)
			shaderStorage.shaders[fragmentShaderIndex].programIndex = -1;
		glDeleteProgram(id);
	}

	bool Shader::Program::attach(size_t shaderIndex) {
		if (shaderIndex <= shaderStorage.shaders.size()) {
			std::cerr << "shaderIndex is out of range" << std::endl;
			return false;
		}

		Shader shader = shaderStorage.shaders[shaderIndex];
		glAttachShader(id, shader.id);

		switch (glGetError()) {
		case GL_INVALID_VALUE:
			if (!glIsShader(shader.id))
				std::cerr << "shader id isn't valid" << std::endl;
			else if (!glIsProgram(id))
				std::cerr << "shader program id isn't valid" << std::endl;
			return false;
		case GL_INVALID_OPERATION:
		{
			if (!glIsProgram(id)) {
				std::cerr << "shader program id isn't valid" << std::endl;
				return false;
			}
			GLsizei actual = 0;
			std::array<GLuint, 10> shaders{};
			glGetAttachedShaders(id, shaders.size(), &actual, shaders.data());
			for (GLsizei i = 0; i < actual; i++) {
				if (shaders[i] == shader.id) {
					std::cerr << "shader was already present in shader program" << std::endl;
					return false;
				}
			}
			if (!glIsShader(shader.id))
				std::cerr << "shader id isn't valid" << std::endl;
			return false;
		}
		}

		GLint shaderType = 0;
		glGetShaderiv(shader.id, GL_SHADER_TYPE, &shaderType);
		switch (shaderType) {
		case GL_VERTEX_SHADER: vertexShaderIndex = shaderIndex; break;
		case GL_FRAGMENT_SHADER: fragmentShaderIndex = shaderIndex; break;
		}

		return true;
	}

	bool Shader::Program::detach(size_t shaderID) {
		glDetachShader(id, shaderStorage.shaders[shaderID].id);
		return true;
	}

	Shader::Shader(GLenum shaderType) : id(glCreateShader(shaderType)), programIndex(-1), ownIndex(-1) {
	}

	Shader::~Shader() {
		if (programIndex != -1)
			shaderStorage.programs[programIndex].detach(ownIndex);
		glDeleteShader(id);
	}

	size_t Shader::load(const std::string_view& source, GLenum shaderType) {
		Shader shader(shaderType);

		if (shader.id == 0) {
			GLenum error = glGetError();
			if (error == GL_INVALID_ENUM)
				std::cerr << "shaderType is not valid, must be one of " <<
				"'GL_VERTEX_SHADER', 'GL_TESS_CONTROL_SHADER', 'GL_TESS_EVALUATION_SHADER', 'GL_GEOMETRY_SHADER', 'GL_FRAGMENT_SHADER', 'GL_COMPUTE_SHADER'"
				<< std::endl;
			else
				std::cerr << "unknown error from glCreateShader" << std::endl;

			return -1;
		}

		if (!shader.setSource(source.data())) return -1;
		if (!shader.compile()) return -1;

		shaderStorage.shaders.push_back(shader);
		shader.ownIndex = shaderStorage.shaders.size();
		return shader.ownIndex;
	}

	size_t Shader::load(std::filesystem::path path, GLenum shaderType) {
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
		std::string str(size, '\0');
		std::ifstream in(path);
		in.read(str.data(), size);

		GLuint shaderID = load(std::string_view(str), shaderType);

		return shaderID;
	}

	size_t Shader::load(const std::filesystem::path& path) {
		std::filesystem::path extension = path.extension();
		if (extension == ".vert") return load(path, GL_VERTEX_SHADER);
		if (extension == ".tesc") return load(path, GL_TESS_CONTROL_SHADER);
		if (extension == ".tese") return load(path, GL_TESS_EVALUATION_SHADER);
		if (extension == ".geom") return load(path, GL_GEOMETRY_SHADER);
		if (extension == ".frag") return load(path, GL_FRAGMENT_SHADER);
		if (extension == ".comp") return load(path, GL_COMPUTE_SHADER);

		std::cerr << "could not refer shader type from path " << path << std::endl;
		return 0;
	}

	bool Shader::setSource(const char* source) {
		glShaderSource(id, 1, &source, nullptr);
		{
			GLint shaderSourceLength = 0;
			glGetShaderiv(id, GL_SHADER_SOURCE_LENGTH, &shaderSourceLength);
			if (shaderSourceLength == 0) {
				std::cerr << "could not upload shader source to OpenGL" << std::endl;

				return false;
			}
		}
		return true;
	}

	bool Shader::compile() {
		glCompileShader(id);
		{
			GLint error = 0;
			glGetShaderiv(id, GL_COMPILE_STATUS, &error);
			if (error == GL_FALSE) {
				int length = 0;
				glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
				std::string errorMessage(length, '\0');
				int writtenCount = 0;
				glGetShaderInfoLog(id, length, &writtenCount, errorMessage.data());
				std::cerr << "shader compiled with error: \n" << errorMessage << std::endl;

				return false;
			}
		}

		return true;
	}
}
