#include "renderObject.h"

#include <iostream>

namespace Minecraft::Assets {
	VBO::VBO() {
		glGenBuffers(1, &vbo);
	}

	VBO::VBO(VBO&& other) noexcept {
		if (vbo)
			glDeleteBuffers(1, &vbo);
	
		this->vbo = other.vbo;
		this->vertexCount = other.vertexCount;

		other.vbo = 0;
		other.vertexCount = 0;
	}

	VBO& VBO::operator=(VBO&& other) noexcept {
		if (this != &other) {
			if (vbo)
				glDeleteBuffers(1, &vbo);

			this->vbo = other.vbo;
			this->vertexCount = other.vertexCount;

			other.vbo = 0;
			other.vertexCount = 0;
		}
		return *this;
	}

	VBO::~VBO() {
		if (vbo)
			glDeleteBuffers(1, &vbo);
	}

	VBO VBO::create(std::function<size_t(GLuint)> vertices) {
		VBO vbo{};

		vbo.bind();
		vbo.vertexCount = vertices(vbo.vbo);
		vbo.unbind();

		return vbo;
	}

	void VBO::bind() {
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
	}

	void VBO::unbind() {
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void VBO::draw(GLenum shape) {
		bind();
		glDrawArrays(shape, 0, vertexCount);
		unbind();
	}
}

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
		obj.vboCount = 1;
		obj.vbos = new GLuint[obj.vboCount];
		glGenBuffers(obj.vboCount, obj.vbos);

		obj.bind();
		glBindBuffer(GL_ARRAY_BUFFER, obj.vbos[0]);
		obj.vertexCount = vertices(obj.vbos[0]);
		obj.unbind();

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		return obj;
	}

	RenderObject RenderObject::create(std::function<size_t(GLuint)> vertices, std::function<void(GLuint)> indices) {
		RenderObject obj = create(vertices);

		glGenBuffers(1, &obj.ebo);
		
		obj.bind();
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj.ebo);
		indices(obj.ebo);
		obj.unbind();

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		return obj;
	}

	RenderObject RenderObject::create(std::function<size_t(GLuint)> vertices, std::vector<GLuint> indices) {
		return create(vertices, [indices](GLuint ebo) {
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
		});
	}

	void RenderObject::bind() {
		glBindVertexArray(vao);
	}

	void RenderObject::unbind() {
		glBindVertexArray(0);
	}

	void RenderObject::draw(GLenum shape) {
		bind();
		if (ebo)
			glDrawElements(shape, vertexCount, GL_UNSIGNED_INT, 0);
		else
			glDrawArrays(shape, 0, vertexCount);
		unbind();
	}
}