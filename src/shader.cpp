#include "shader.h"

#include <glm/gtc/type_ptr.hpp>

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
		if (!path)
			return;

		if (std::filesystem::last_write_time(*path) >= lastTimeStamp)
			return;

		std::shared_ptr<Shader> other = parse(*path);
		if (!other) {
			std::cerr << "Failed to recompile changed shader, keeping old one" << std::endl;
			return;
		}

		std::swap(*this, *other);

		std::vector<GLuint> attachedShaders;

		std::erase_if(programs, [this, &attachedShaders](std::weak_ptr<ShaderProgram> _program) {
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

		for (auto& _program : programs) {
			if (auto program = _program.lock()) {
				glDetachShader(program->id, other->id);
				glAttachShader(program->id, this->id);

				program->link();
			}
		}
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

namespace Minecraft::Assets {
	ShaderProgram::ShaderProgram() : id(glCreateProgram()) {
		if (id == 0)
			std::cerr << "unknown error from glCreateProgram" << std::endl;
	}

	ShaderProgram::ShaderProgram(ShaderProgram&& other) noexcept {
		if (id != 0)
			glDeleteProgram(id);

		this->id = other.id;
		this->shaders = other.shaders;

		other.id = 0;
		other.shaders = {};
	}

	ShaderProgram& ShaderProgram::operator=(ShaderProgram&& other) noexcept {
		if (this != &other) {
			if (id != 0)
				glDeleteProgram(id);

			this->id = other.id;
			this->shaders = other.shaders;

			other.id = 0;
			other.shaders = {};
		}
		return *this;
	}

	ShaderProgram::~ShaderProgram() {
		if (id != 0)
			glDeleteProgram(id);
	}

	std::shared_ptr<ShaderProgram> ShaderProgram::create() {
		return std::shared_ptr<ShaderProgram>(new ShaderProgram());
	}

	std::shared_ptr<ShaderProgram> ShaderProgram::attachShader(std::weak_ptr<Shader> _shader) {
		if (auto shader = _shader.lock()) {
			GLint shaderCount = 0;
			glGetProgramiv(id, GL_ATTACHED_SHADERS, &shaderCount);
			std::vector<GLuint> attachedShaders(shaderCount);
			glGetAttachedShaders(id, shaderCount, &shaderCount, attachedShaders.data());
			if (std::none_of(attachedShaders.begin(), attachedShaders.end(), [shader](GLuint i) { return i == shader->id; })) {
				glAttachShader(id, shader->id);
				{
					GLenum error = glGetError();
					if (error == GL_INVALID_VALUE) {
						if (!glIsProgram(id))
							std::cerr << "provided program '" << id << "' isn't known to opengl" << std::endl;
						if (!glIsShader(shader->id))
							std::cerr << "provided shader '" << shader->id << "' isn't known to opengl" << std::endl;

						return shared_from_this();
					} else if (error == GL_INVALID_OPERATION) {
						if (!glIsProgram(id))
							std::cerr << "provided program '" << id << "' isn't a program" << std::endl;
						if (!glIsShader(shader->id))
							std::cerr << "provided shader '" << shader->id << "' isn't a shader" << std::endl;
						// error code is technicly possible when the shader is already attached to the program,
						// but we explicitly check for it not being the case

						return shared_from_this();
					}
				}
				shader->programs.push_back(weak_from_this());
				shaders.push_back(shader);
			}
		}

		return shared_from_this();
	}

	std::shared_ptr<ShaderProgram> ShaderProgram::detachShader(std::weak_ptr<Shader> _shader) {
		if (auto shader = _shader.lock()) {
			GLint shaderCount = 0;
			glGetProgramiv(id, GL_ATTACHED_SHADERS, &shaderCount);
			std::vector<GLuint> attachedShaders(shaderCount);
			glGetAttachedShaders(id, shaderCount, &shaderCount, attachedShaders.data());
			if (std::any_of(attachedShaders.begin(), attachedShaders.end(), [shader](GLuint i) { return i == shader->id; })) {
				glDetachShader(id, shader->id);
				{
					GLenum error = glGetError();
					if (error == GL_INVALID_VALUE) {
						if (!glIsProgram(id))
							std::cerr << "provided program '" << id << "' isn't known to opengl" << std::endl;
						if (!glIsShader(shader->id))
							std::cerr << "provided shader '" << shader->id << "' isn't known to opengl" << std::endl;

						return shared_from_this();
					} else if (error == GL_INVALID_OPERATION) {
						if (!glIsProgram(id))
							std::cerr << "provided program '" << id << "' isn't a program" << std::endl;
						if (!glIsShader(shader->id))
							std::cerr << "provided shader '" << shader->id << "' isn't a shader" << std::endl;
						// error code is technicly possible when the shader is already attached to the program,
						// but we explicitly check for it not being the case

						return shared_from_this();
					}
				}
				std::shared_ptr<ShaderProgram> thisPtr;
				std::erase_if(shader->programs, [thisPtr](const std::weak_ptr<ShaderProgram>& _program) {
					if (auto program = _program.lock())
						return program == thisPtr;
					return false;
				});
				std::erase(shaders, shader);
			}
		}

		return shared_from_this();
	}

	std::shared_ptr<ShaderProgram> ShaderProgram::bindAttribute(GLuint index, const std::string& name) {
		if (name.starts_with("gl_")) {
			std::cerr << "attribute '" << name << "' starts with reserved prefix 'gl_'" << std::endl;
			return shared_from_this();
		}
		if (index >= GL_MAX_VERTEX_ATTRIBS) {
			std::cerr << "index '" << index << "' is out of range [0, " << GL_MAX_VERTEX_ATTRIBS << ")" << std::endl;
			return shared_from_this();
		}

		glBindAttribLocation(id, index, name.c_str());

		return shared_from_this();
	}

	std::shared_ptr<ShaderProgram> ShaderProgram::link() {
		glLinkProgram(id);

		{
			GLint error = 0;
			glGetProgramiv(id, GL_LINK_STATUS, &error);

			if (error == GL_FALSE) {
				int length = 0;
				glGetProgramiv(id, GL_INFO_LOG_LENGTH, &length);
				std::string errorMessage(length, '\0');
				int writtenCount = 0;
				glGetProgramInfoLog(id, length, &writtenCount, errorMessage.data());
				std::cerr << "shader program linked with error:\n" << errorMessage << std::endl;
			}
		}

		return shared_from_this();
	}

	std::shared_ptr<ShaderProgram> ShaderProgram::use() {
		GLint isLinked = 0;
		glGetProgramiv(id, GL_LINK_STATUS, &isLinked);

		if (isLinked == GL_TRUE)
			glUseProgram(id);
		else
			std::cerr << "Attempted to use unlinked program" << std::endl;

		return shared_from_this();
	}

	void ShaderProgram::update() {
		for (auto& shader : shaders)
			shader->update();
	}

	GLint ShaderProgram::getUniform(const std::string& uniform) {
		return glGetUniformLocation(id, uniform.c_str());
	}

	void ShaderProgram::setUniform(GLint uniform, bool value) {
		glUniform1i(uniform, value ? GL_TRUE : GL_FALSE);
	}

	void ShaderProgram::setUniform(GLint uniform, int value) {
		glUniform1i(uniform, value);
	}

	void ShaderProgram::setUniform(GLint uniform, float value) {
		glUniform1f(uniform, value);
	}

	void ShaderProgram::setUniform(GLint uniform, const glm::mat4& value) {
		glUniformMatrix4fv(uniform, 1, GL_FALSE, glm::value_ptr(value));
	}

	void ShaderProgram::setUniform(GLint uniform, const glm::mat3& value) {
		glUniformMatrix3fv(uniform, 1, GL_FALSE, glm::value_ptr(value));
	}

	void ShaderProgram::setUniform(GLint uniform, const glm::vec4& value) {
		glUniform4fv(uniform, 1, glm::value_ptr(value));
	}

	void ShaderProgram::setUniform(GLint uniform, const glm::vec3& value) {
		glUniform3fv(uniform, 1, glm::value_ptr(value));
	}

	void ShaderProgram::setUniform(GLint uniform, const glm::vec2& value) {
		glUniform2fv(uniform, 1, glm::value_ptr(value));
	}
}

//Shader::Shader(const std::string& path, GLenum shaderType) {
//	{
//		std::string roughPath = path;
//		if (!parsePath(roughPath, shaderType))
//			throw std::runtime_error(roughPath);
//		this->path = roughPath;
//	}
//
//	std::string shaderString = readEntireFile();
//
//	if (!compile(shaderString.c_str(), shaderType))
//		throw std::runtime_error(lastError);
//}
//
//bool Shader::shouldUpdate() const {
//	if (!path.has_value()) {
//		return false;
//	}
//
//	struct stat result{};
//
//	if (stat(path->c_str(), &result) == -1) {
//		std::cerr << "filepath somehow invalid" << std::endl << "ignoring update" << std::endl;
//		return false;
//	}
//
//	bool retVal = false;
//	time_t fileChangedTimeStamp = result.st_mtime;
//	if (fileChangedTimeStamp > lastTimeStamp)
//		retVal = true;
//	lastTimeStamp = fileChangedTimeStamp;
//
//	return retVal;
//}
//
//bool Shader::compile(const char* source, GLenum shaderType) {
//	if (id != -1)
//		glDeleteShader(id);
//
//	switch (shaderType) {
//		case GL_FRAGMENT_SHADER:
//			id = glCreateShader(GL_FRAGMENT_SHADER);
//			break;
//		case GL_VERTEX_SHADER:
//			id = glCreateShader(GL_VERTEX_SHADER);
//			break;
//		default:
//			lastError = "shader type not supported";
//			return false;
//	}
//	if (glGetError() == GL_INVALID_ENUM) {
//		lastError = "passed in wrong shader type";
//		id = -1;
//		return false;
//	}
//	if (id == 0) {
//		lastError = "internal OpenGL error";
//		id = -1;
//		return false;
//	}
//
//	glShaderSource(id, 1, &source, nullptr);
//	{
//		GLint error = 0;
//		glGetShaderiv(id, GL_SHADER_SOURCE_LENGTH, &error);
//		if (error == 0) {
//			lastError = "could not upload shader source to OpenGL";
//			glDeleteShader(id);
//			id = -1;
//			return false;
//		}
//	}
//
//	glCompileShader(id);
//	{
//		GLint error = 0;
//		glGetShaderiv(id, GL_COMPILE_STATUS, &error);
//		if (error == GL_FALSE) {
//			int length = 0;
//			glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
//			char* errorMessage = new char[length + 1];
//			memset(errorMessage, 0, length + 1);
//			int writtenCount = 0;
//			glGetShaderInfoLog(id, length, &writtenCount, errorMessage);
//			lastError = std::string(errorMessage);
//			delete[] errorMessage;
//
//			glDeleteShader(id);
//			return false;
//		}
//	}
//
//	return true;
//}
//
//bool Shader::parsePath(std::string& pathToParse, GLenum shaderType) {
//	if (pathToParse.find('\\') == std::string::npos && pathToParse.find('/') == std::string::npos)
//		pathToParse = "assets/shaders/" + pathToParse;
//	if (pathToParse.find('.') == std::string::npos) {
//		switch (shaderType) {
//			case GL_FRAGMENT_SHADER:
//				pathToParse += ".frag";
//				break;
//			case GL_VERTEX_SHADER:
//				pathToParse += ".vert";
//				break;
//			default:
//				pathToParse = "";
//				lastError = "shader type not supported";
//				return false;
//		}
//	}
//
//	return true;
//}
//
//std::string Shader::readEntireFile() {
//	struct stat result{};
//
//	if (stat(path->c_str(), &result) == -1) {
//		lastError = "file path '" + path.value() + "' invalid";
//		throw std::runtime_error(lastError);
//	}
//
//	std::ifstream file(path->c_str());
//
//	if (!file) {
//		lastError = "file path '" + path.value() + "' invalid";
//		throw std::runtime_error(lastError);
//	}
//
//	std::string shaderString((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
//
//	if (shaderString.empty()) {
//		lastError = "file wasn't loaded";
//		throw std::runtime_error(lastError);
//	}
//
//	lastTimeStamp = result.st_mtime;
//
//	return shaderString;
//}
