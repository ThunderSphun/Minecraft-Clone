#include "renderObject.h"

#include <iostream>

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

	EBO EBO::create(const std::function<size_t(GLuint)>& indices) {
		EBO ebo{};

		ebo.bind();
		ebo.indicesCount = indices(ebo.ebo);
		ebo.unbind();

		return ebo;
	}

	EBO EBO::create(const std::vector<GLuint>& indices) {
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

	size_t EBO::getSize() const {
		return indicesCount;
	}
}
