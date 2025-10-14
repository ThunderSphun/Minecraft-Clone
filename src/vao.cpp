#include "renderObject.h"

#include <iostream>

namespace Minecraft::Assets {
	VAO::VAO() {
		glGenVertexArrays(1, &vao);
	}

	VAO::VAO(VAO&& other) noexcept {
		if (vao != 0)
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
			if (vao != 0)
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
		if (vao != 0)
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
			return create(vbos[0], ebo);

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
		{
			GLenum error = glGetError();
			if (error == GL_INVALID_ENUM) {
				std::cerr << "Rendershape is not valid, must be one of " <<
					"'GL_POINTS', 'GL_LINE_STRIP', 'GL_LINE_LOOP', 'GL_LINES', 'GL_TRIANGLE_STRIP', 'GL_TRIANGLE_FAN', 'GL_TRIANGLES', 'GL_PATCHES'";
				int version[] = { 3, 0 };
				glGetIntegerv(GL_MAJOR_VERSION, &version[0]);
				glGetIntegerv(GL_MINOR_VERSION, &version[1]);
				if ((version[0] == 3 && version[1] >= 2) || version[0] > 3)
					std::cerr << ", 'GL_LINE_STRIP_ADJACENCY', 'GL_LINES_ADJACENCY', 'GL_TRIANGLE_STRIP_ADJACENCY', 'GL_TRIANGLES_ADJACENCY'";

				std::cerr << std::endl;
			} else if (error == GL_INVALID_OPERATION) {
				bool isMapped = false;
				glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_MAPPED, (int*) &isMapped);

				if (isMapped)
					std::cerr << "Buffer is currently mapped for data access, and cannot be rendered" << std::endl;
				else {
					GLenum geometryShape = 0;
					GLint currentProgram = 0;

					glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
					glGetProgramiv(currentProgram, GL_GEOMETRY_INPUT_TYPE, (int*) &geometryShape);
					if (geometryShape != shape)
						std::cerr << "A geometry shader is present, and requires a certain rendershape" << std::endl;
					else
						std::cerr << (ebo ? "'glDrawElements'" : "'glDrawArrays'") << " caused 'GL_INVALID_OPERATION' error with unknown cause" << std::endl;
				}
			} else if (error != GL_NO_ERROR)
				std::cerr << (ebo ? "'glDrawElements'" : "'glDrawArrays'") << " caused unchecked error" << std::endl;
		}
		unbind();
	}
}
