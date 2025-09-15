#pragma once

#include <GL/glew.h>

#include <filesystem>
#include <string_view>

namespace Assets {
	class Shader {
	public:
		class Program {
		public:
			Program();
			~Program();

			bool attach(size_t shaderID);
			bool detach(size_t shaderID);
			bool linkShader();
			bool use();

		private:
			size_t vertexShaderIndex;
			size_t fragmentShaderIndex;
			size_t ownIndex;
			GLuint id;
		};

		friend Program;

		static size_t load(const std::string_view& source, GLenum shaderType);
		static size_t load(std::filesystem::path path, GLenum shaderType);
		static size_t load(const std::filesystem::path& path);

		bool setSource(const char* source);
		bool compile();

	private:
		explicit Shader(GLenum shaderType);
		~Shader();
		
		size_t programIndex;
		size_t ownIndex;
		GLuint id;
	};

	static struct ShaderStorage {
		std::vector<Shader> shaders;
		std::vector<Shader::Program> programs;
	} shaderStorage;
}
