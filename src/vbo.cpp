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

	size_t VBO::getSize() {
		return vertexCount;
	}

	void VBO::draw(GLenum shape) {
		bind();
		glDrawArrays(shape, 0, vertexCount);
		unbind();
	}
}
