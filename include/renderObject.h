#pragma once

#include <GL/glew.h>

#include <functional>
#include <vector>

namespace Minecraft::Assets {
	class RenderObject {
	public:
		RenderObject(const RenderObject&) = delete;
		RenderObject(RenderObject&& other) noexcept;
		RenderObject& operator=(const RenderObject&) = delete;
		RenderObject& operator=(RenderObject&& other) noexcept;
		~RenderObject();

		static RenderObject create(std::function<size_t(GLuint)> vertices);
		static RenderObject create(std::function<size_t(GLuint)> vertices, std::function<void(GLuint)> indices);
		static RenderObject create(std::function<size_t(GLuint)> vertices, std::vector<GLuint> indices);

		void bind();
		void unbind();

		void draw();

	private:
		RenderObject() = default;

		GLuint vao = 0;
		GLuint* vbos = nullptr;
		GLsizei vboCount = 0;
		GLuint ebo = 0;

		size_t vertexCount = 0;
	};
}