#include "renderObject.h"

namespace Minecraft::Assets {
	RenderObject::RenderObject(RenderObject&& other) noexcept {
		if (vao)
			glDeleteVertexArrays(1, &vao);
		if (vbos) {
			glDeleteBuffers(vboCount, vbos);
			delete[] vbos;
		}
		if (ebo)
			glDeleteBuffers(1, &ebo);

		this->vao = other.vao;
		this->vbos = other.vbos;
		this->vboCount = other.vboCount;
		this->ebo = other.ebo;
		this->vertexCount = other.vertexCount;

		other.vao = 0;
		other.vbos = nullptr;
		other.vboCount = 0;
		other.ebo = 0;
		other.vertexCount = 0;
	}

	RenderObject& RenderObject::operator=(RenderObject&& other) noexcept {
		if (this != &other) {
			if (vao)
				glDeleteVertexArrays(1, &vao);
			if (vbos) {
				glDeleteBuffers(vboCount, vbos);
				delete[] vbos;
			}
			if (ebo)
				glDeleteBuffers(1, &ebo);

			this->vao = other.vao;
			this->vbos = other.vbos;
			this->vboCount = other.vboCount;
			this->ebo = other.ebo;
			this->vertexCount = other.vertexCount;

			other.vao = 0;
			other.vbos = nullptr;
			other.vboCount = 0;
			other.ebo = 0;
			other.vertexCount = 0;
		}
		return *this;
	}

	RenderObject::~RenderObject() {
		if (vao)
			glDeleteVertexArrays(1, &vao);
		if (vbos) {
			glDeleteBuffers(vboCount, vbos);
			delete[] vbos;
		}
		if (ebo)
			glDeleteBuffers(1, &ebo);
	}

	RenderObject RenderObject::create(std::function<size_t(GLuint)> vertices) {
		RenderObject obj{};

		glGenVertexArrays(1, &obj.vao);
		glBindVertexArray(obj.vao);

		obj.vboCount = 1;
		obj.vbos = new GLuint[obj.vboCount];
		glGenBuffers(obj.vboCount, obj.vbos);
		glBindBuffer(GL_ARRAY_BUFFER, obj.vbos[0]);

		obj.vertexCount = vertices(obj.vbos[0]);

		obj.unbind();

		return std::move(obj);
	}

	RenderObject RenderObject::create(std::function<size_t(GLuint)> vertices, std::function<void(GLuint)> indices) {
		RenderObject obj = create(vertices);

		obj.bind();

		glGenBuffers(1, &obj.ebo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj.ebo);
		indices(obj.ebo);

		obj.unbind();
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		return std::move(obj);
	}

	RenderObject RenderObject::create(std::vector<std::function<size_t(GLuint*)>> vertices) {
		if (vertices.size() == 0)
			return RenderObject{};
		if (vertices.size() == 1)
			return create([vertices](GLuint vbo) -> size_t { return vertices[0](&vbo); });

		RenderObject obj{};

		glGenVertexArrays(1, &obj.vao);
		glBindVertexArray(obj.vao);

		obj.vboCount = vertices.size();
		obj.vbos = new GLuint[obj.vboCount];
		glGenBuffers(obj.vboCount, obj.vbos);

		for (size_t i = 0; i < obj.vboCount; i++) {
			glBindBuffer(GL_ARRAY_BUFFER, obj.vbos[i]);
			obj.vertexCount = vertices[i](obj.vbos);
		}

		obj.unbind();

		return std::move(obj);
	}

	RenderObject RenderObject::create(std::vector<std::function<size_t(GLuint*)>> vertices, std::function<void(GLuint)> indices) {
		RenderObject obj = create(vertices);

		obj.bind();

		glGenBuffers(1, &obj.ebo);
		indices(obj.ebo);

		obj.unbind();
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		return std::move(obj);
	}

	void RenderObject::bind() {
		glBindVertexArray(vao);
	}

	void RenderObject::unbind() {
		glBindVertexArray(0);
	}

	void RenderObject::draw() {
		bind();
		if (ebo)
			glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, 0);
		else
			glDrawArrays(GL_TRIANGLES, 0, vertexCount);
		unbind();
	}
}