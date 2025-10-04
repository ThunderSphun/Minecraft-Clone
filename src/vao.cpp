#include "renderObject.h"

#include <iostream>

namespace Minecraft::Assets {
	VAO::VAO() {
		glGenVertexArrays(1, &vao);
	}

	VAO::VAO(VAO&& other) noexcept {
		if (vao)
			glDeleteVertexArrays(1, &vao);

		this->vao = other.vao;
		this->vbos = std::move(other.vbos);
		this->ebo = std::move(other.ebo);

		other.vao = 0;
		other.vbos.clear();
		other.ebo = {};
	}

	VAO& VAO::operator=(VAO&& other) noexcept {
		if (this != &other) {
			if (vao)
				glDeleteVertexArrays(1, &vao);

			this->vao = other.vao;
			this->vbos = std::move(other.vbos);
			this->ebo = std::move(other.ebo);

			other.vao = 0;
			other.vbos.clear();
			other.ebo = {};
		}
		return *this;
	}

	VAO::~VAO() {
		if (vao)
			glDeleteVertexArrays(1, &vao);
	}

	VAO VAO::create(const std::function<VBO()>& vbo) {
		VAO vao{};

		vao.bind();

		vao.vbos.push_back(vbo());

		vao.vbos[0].bind();

		vao.unbind();
		vao.vbos[0].unbind();

		return vao;
	}

	VAO VAO::create(const std::function<VBO()>& vbo, const std::function<EBO()>& ebo) {
		VAO vao{};

		vao.bind();

		vao.vbos.push_back(vbo());
		vao.ebo = ebo();

		vao.vbos[0].bind();
		vao.ebo->bind();

		vao.unbind();
		vao.vbos[0].unbind();
		vao.ebo->unbind();

		return vao;
	}

	VAO Minecraft::Assets::VAO::create(const std::vector<std::function<VBO()>>& vbos) {
		if (vbos.size() == 0)
			return VAO{};
		if (vbos.size() == 1)
			return create(vbos[0]);

		VAO vao{};

		vao.bind();

		vao.vbos.reserve(vbos.size());
		for (auto& vbo : vbos) {
			vao.vbos.push_back(vbo());
			vao.vbos.back().bind();
		}

		vao.unbind();
		for (auto& vbo : vao.vbos)
			vbo.unbind();

		return vao;
	}
	
	VAO Minecraft::Assets::VAO::create(const std::vector<std::function<VBO()>>& vbos, const std::function<EBO()>& ebo) {
		if (vbos.size() == 0)
			return VAO{};
		if (vbos.size() == 1)
			return create(vbos[0]);

		VAO vao{};

		vao.bind();

		vao.vbos.reserve(vbos.size());
		for (auto& vbo : vbos) {
			vao.vbos.push_back(vbo());
			vao.vbos.back().bind();
		}
		vao.ebo = ebo();
		vao.ebo->bind();

		vao.unbind();
		for (auto& vbo : vao.vbos)
			vbo.unbind();
		vao.ebo->unbind();

		return vao;
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
			glDrawElements(shape, ebo->getSize(), GL_UNSIGNED_INT, 0);
		else
			glDrawArrays(shape, 0, vbos[0].getSize());
		unbind();
	}
}
