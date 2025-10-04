#pragma once

#include <GL/glew.h>

#include <functional>
#include <vector>
#include <optional>

namespace Minecraft::Assets {
	class VBO {
	public:
		VBO(const VBO&) = delete;
		VBO(VBO&& other) noexcept;
		VBO& operator=(const VBO&) = delete;
		VBO& operator=(VBO&& other) noexcept;
		~VBO();

		static VBO create(std::function<size_t(GLuint)> vertices);

		void bind();
		void unbind();

		size_t getSize();

		/// draws the vbo
		/// only works with opengl in compat mode
		void draw(GLenum shape = GL_TRIANGLES);

	private:
		VBO();

		GLuint vbo = 0;

		size_t vertexCount = 0;
	};

	class EBO {
	public:
		EBO(const EBO&) = delete;
		EBO(EBO&& other) noexcept;
		EBO& operator=(const EBO&) = delete;
		EBO& operator=(EBO&& other) noexcept;
		~EBO();

		static EBO create(std::function<size_t(GLuint)> indices);
		static EBO create(std::vector<GLuint> indices);

		void bind();
		void unbind();

		size_t getSize();

	private:
		EBO();

		GLuint ebo = 0;

		size_t indicesCount = 0;
	};

	class VAO {
	public:
		VAO(const VAO&) = delete;
		VAO(VAO&& other) noexcept;
		VAO& operator=(const VAO&) = delete;
		VAO& operator=(VAO&& other) noexcept;
		~VAO();

		static VAO create(std::function<VBO()> vbo);
		static VAO create(std::function<VBO()> vbo, std::function<EBO()> ebo);
		static VAO create(std::vector<std::function<VBO()>> vbos);
		static VAO create(std::vector<std::function<VBO()>> vbos, std::function<EBO()> ebo);

		void bind();
		void unbind();

		void draw(GLenum shape = GL_TRIANGLES);

	private:
		VAO();

		GLuint vao = 0;
		std::vector<VBO> vbos;
		std::optional<EBO> ebo;
	};
}
