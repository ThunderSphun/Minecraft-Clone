#include "shader.h"

#include <iostream>
#include <fstream>
#include <algorithm>

namespace Minecraft::Assets {
	Shader::Shader(GLenum shaderType) : id(glCreateShader(shaderType)) {
		if (id == 0) {
			GLenum error = glGetError();
			if (error == GL_INVALID_ENUM)
				std::cerr << "shaderType is not valid, must be one of " <<
				"'GL_VERTEX_SHADER', 'GL_TESS_CONTROL_SHADER', 'GL_TESS_EVALUATION_SHADER', 'GL_GEOMETRY_SHADER', 'GL_FRAGMENT_SHADER', 'GL_COMPUTE_SHADER'"
				<< std::endl;
			else
				std::cerr << "unknown error from glCreateShader" << std::endl;
		}
	}

	Shader::Shader(Shader&& other) noexcept {
		if (id != 0)
			glDeleteShader(id);

		this->id = other.id;
		this->lastTimeStamp = other.lastTimeStamp;
		this->path = other.path;
		this->programs = other.programs;

		other.id = 0;
		other.lastTimeStamp = std::filesystem::file_time_type::min();
		other.path = {};
		other.programs = {};
	}

	Shader& Shader::operator=(Shader&& other) noexcept {
		if (this != &other) {
			if (id != 0)
				glDeleteShader(id);

			this->id = other.id;
			this->lastTimeStamp = other.lastTimeStamp;
			this->path = other.path;
			this->programs = other.programs;

			other.id = 0;
			other.lastTimeStamp = std::filesystem::file_time_type::min();
			other.path = {};
			other.programs = {};
		}
		return *this;
	}

	Shader::~Shader() {
		if (id != 0)
			glDeleteShader(id);
	}

	void Shader::update() {
		if (path) {
			std::filesystem::file_time_type lastWriteTime = std::filesystem::last_write_time(*path);
			if (lastWriteTime > lastTimeStamp) {
				std::shared_ptr<Shader> other = parse(*path);
				if (!other) {
					std::cerr << "Failed to recompile changed shader, keeping old one" << std::endl;
					lastTimeStamp = lastWriteTime;
				} else {
					std::swap(*this, *other);
					std::swap(this->programs, other->programs);

					for (const std::weak_ptr<Program>& _program : programs) {
						if (auto program = _program.lock()) {
							if (std::any_of(program->shaders.begin(), program->shaders.end(), [this](const std::shared_ptr<Shader>& shader) { return shader->id == id; })) {
								glDetachShader(program->id, other->id);
								glAttachShader(program->id, this->id);

								program->link();
							}
						}
					}
				}
			}
		}

		std::erase_if(programs, [this](std::weak_ptr<Shader::Program> _program) {
			auto program = _program.lock();
			if (!program)
				return true;

			{
				GLint param = 0;
				glGetProgramiv(program->id, GL_DELETE_STATUS, &param);
				if (param == GL_TRUE)
					return true;
			}

			return false;
		});
	}

	bool Shader::loadShaderSource(const std::string& source) {
		const char* sourceData = source.c_str();

		glShaderSource(id, 1, &sourceData, nullptr);
		{
			GLint shaderSourceLength = 0;
			glGetShaderiv(id, GL_SHADER_SOURCE_LENGTH, &shaderSourceLength);
			if (shaderSourceLength == 0) {
				std::cerr << "could not upload shader source to OpenGL" << std::endl;

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
				std::string errorMessage(length, '\0');
				int writtenCount = 0;
				glGetShaderInfoLog(id, length, &writtenCount, errorMessage.data());
				std::cerr << "shader compiled with error:\n" << errorMessage << std::endl;

				return false;
			}
		}

		return true;
	}

	bool Shader::loadShaderSource(const std::vector<std::string>& sources) {
		size_t sourcesSize = sources.size();
		if (sourcesSize == 1)
			return loadShaderSource(sources[0]);
		const char** sourceData = new const char*[sourcesSize];
		for (size_t i = 0; i < sourcesSize; i++)
			sourceData[i] = sources[i].c_str();
		glShaderSource(id, sourcesSize, sourceData, nullptr);
		delete[] sourceData;
		{
			GLint shaderSourceLength = 0;
			glGetShaderiv(id, GL_SHADER_SOURCE_LENGTH, &shaderSourceLength);
			if (shaderSourceLength == 0) {
				std::cerr << "could not upload shader source to OpenGL" << std::endl;

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
				std::string errorMessage(length, '\0');
				int writtenCount = 0;
				glGetShaderInfoLog(id, length, &writtenCount, errorMessage.data());
				std::cerr << "shader compiled with error:\n" << errorMessage << std::endl;

				return false;
			}
		}

		return true;
	}

	std::shared_ptr<Shader> Shader::parse(std::filesystem::path path) {
		std::filesystem::path extension = path.extension();
		if (extension == ".vert") return parse(path, GL_VERTEX_SHADER);
		if (extension == ".tesc") return parse(path, GL_TESS_CONTROL_SHADER);
		if (extension == ".tese") return parse(path, GL_TESS_EVALUATION_SHADER);
		if (extension == ".geom") return parse(path, GL_GEOMETRY_SHADER);
		if (extension == ".frag") return parse(path, GL_FRAGMENT_SHADER);
		if (extension == ".comp") return parse(path, GL_COMPUTE_SHADER);

		std::cerr << "could not refer shader type from path '" << path << "'" << std::endl;
		return std::shared_ptr<Shader>(nullptr);
	}

	std::shared_ptr<Shader> Shader::parse(std::filesystem::path path, GLenum shaderType) {
		if (path.empty()) return std::shared_ptr<Shader>(nullptr);
		if (!path.has_extension()) {
			switch (shaderType) {
			case GL_VERTEX_SHADER: path += ".vert"; break;
			case GL_TESS_CONTROL_SHADER: path += ".tesc"; break;
			case GL_TESS_EVALUATION_SHADER: path += ".tese"; break;
			case GL_GEOMETRY_SHADER: path += ".geom"; break;
			case GL_FRAGMENT_SHADER: path += ".frag"; break;
			case GL_COMPUTE_SHADER: path += ".comp"; break;
			default:
				std::cerr << "could not defer extension from shaderType '" << shaderType << "'" << std::endl;
				return std::shared_ptr<Shader>(nullptr);
			}
		}
		if (path.has_parent_path() && path.parent_path() == "shaders")
			path = std::filesystem::path("assets") / path;
		else if (!path.has_parent_path())
			path = std::filesystem::path("assets") / "shaders" / path;

		if (!std::filesystem::exists(path)) {
			std::cout << "file '" << path.filename() << "' at '" << path.parent_path() << "' does not exist" << std::endl;
			return std::shared_ptr<Shader>(nullptr);
		}

		uintmax_t size = std::filesystem::file_size(path);
		std::string str(size, '\0');
		std::ifstream in(path);
		in.read(str.data(), size);

		std::shared_ptr<Shader> shader = parse(str, shaderType);
		if (!shader)
			return std::shared_ptr<Shader>(nullptr);

		shader->path = path;
		shader->lastTimeStamp = std::filesystem::last_write_time(*shader->path);
		return shader;
	}

	std::shared_ptr<Shader> Shader::parse(const std::string& source, GLenum shaderType) {
		std::shared_ptr<Shader> shader = std::make_shared<Shader>(shaderType);

		if (shader->loadShaderSource(source))
			return shader;
		else
			return std::shared_ptr<Shader>(nullptr);
	}
}
