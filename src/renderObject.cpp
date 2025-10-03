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
	EBO::EBO() {
		glGenBuffers(1, &ebo);
	}

	EBO::EBO(EBO&& other) noexcept {
		if (ebo)
			glDeleteBuffers(1, &ebo);

		this->ebo = other.ebo;
		this->indicesCount = other.indicesCount;

		other.ebo = 0;
		other.indicesCount = 0;
	}

	EBO& EBO::operator=(EBO&& other) noexcept {
		if (this != &other) {
			if (ebo)
				glDeleteBuffers(1, &ebo);

			this->ebo = other.ebo;
			this->indicesCount = other.indicesCount;

			other.ebo = 0;
			other.indicesCount = 0;
		}
		return *this;
	}

	EBO::~EBO() {
		if (ebo)
			glDeleteBuffers(1, &ebo);
	}

	EBO EBO::create(std::function<size_t(GLuint)> indices) {
		EBO ebo{};

		ebo.bind();
		ebo.indicesCount = indices(ebo.ebo);
		ebo.unbind();

		return ebo;
	}

	EBO EBO::create(std::vector<GLuint> indices) {
		return create([indices](GLuint ebo) {
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

			return indices.size();
		});
	}

	void EBO::bind() {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	}

	void EBO::unbind() {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
}

namespace Minecraft::Assets {
	VAO::VAO(VAO&& other) noexcept {
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

	VAO& VAO::operator=(VAO&& other) noexcept {
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

	VAO::~VAO() {
		if (vao)
			glDeleteVertexArrays(1, &vao);
		if (vbos) {
			glDeleteBuffers(vboCount, vbos);
			delete[] vbos;
		}
		if (ebo)
			glDeleteBuffers(1, &ebo);
	}

	VAO VAO::create(std::function<size_t(GLuint)> vertices) {
		VAO obj{};

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

	VAO VAO::create(std::function<size_t(GLuint)> vertices, std::function<void(GLuint)> indices) {
		VAO obj = create(vertices);

		glGenBuffers(1, &obj.ebo);
		
		obj.bind();
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj.ebo);
		indices(obj.ebo);
		obj.unbind();

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		return obj;
	}

	VAO VAO::create(std::function<size_t(GLuint)> vertices, std::vector<GLuint> indices) {
		return create(vertices, [indices](GLuint ebo) {
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
		});
	}

	void VAO::bind() {
		glBindVertexArray(vao);
	}

	void VAO::unbind() {
		glBindVertexArray(0);
	}

	void VAO::draw(GLenum shape) {
		bind();
		if (ebo)
			glDrawElements(shape, vertexCount, GL_UNSIGNED_INT, 0);
		else
			glDrawArrays(shape, 0, vertexCount);
		unbind();
	}
}