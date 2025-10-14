#include "renderObject.h"

#include <iostream>

namespace Minecraft::Assets {
	VBO::VBO() {
		glGenBuffers(1, &vbo);
	}

	VBO::VBO(VBO&& other) noexcept {
		if (vbo != 0)
			glDeleteBuffers(1, &vbo);

		this->vbo = other.vbo;
		this->vertexCount = other.vertexCount;

		other.vbo = 0;
		other.vertexCount = 0;
	}

	VBO& VBO::operator=(VBO&& other) noexcept {
		if (this != &other) {
			if (vbo != 0)
				glDeleteBuffers(1, &vbo);

			this->vbo = other.vbo;
			this->vertexCount = other.vertexCount;

			other.vbo = 0;
			other.vertexCount = 0;
		}
		return *this;
	}

	VBO::~VBO() {
		if (vbo != 0)
			glDeleteBuffers(1, &vbo);
	}

	VBO VBO::create(const std::function<size_t(GLuint)>& vertices) {
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

	size_t VBO::getSize() const {
		return vertexCount;
	}

	void VBO::draw(GLenum shape) {
		bind();
		glDrawArrays(shape, 0, vertexCount);
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
						std::cerr << "'glDrawArrays' caused 'GL_INVALID_OPERATION' error with unknown cause" << std::endl;
				}
			} else if (error != GL_NO_ERROR)
				std::cerr << "'glDrawArrays' caused unchecked error" << std::endl;
		}
		unbind();
	}
}
