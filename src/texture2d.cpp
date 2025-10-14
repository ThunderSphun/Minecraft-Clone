#include "texture.h"

#include <stb_image.h>

#include <fstream>
#include <iostream>

namespace Minecraft::Assets {
	Texture2D::Texture2D(const uint8_t* data, int width, int height) : size({ width, height }) {
		glGenTextures(1, &texture);

		GLint previousTexture = 0;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &previousTexture);
		bind();

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		if (previousTexture != 0)
			glBindTexture(GL_TEXTURE_2D, previousTexture);
	}

	Texture2D::Texture2D(Texture2D&& other) noexcept {
		if (texture != 0)
			glDeleteTextures(1, &texture);

		this->texture = other.texture;
		this->size = std::move(other.size);

		other.texture = 0;
		other.size = { 0, 0 };
	}

	Texture2D& Texture2D::operator=(Texture2D&& other) noexcept {
		if (this != &other) {
			if (texture != 0)
				glDeleteTextures(1, &texture);

			this->texture = other.texture;
			this->size = std::move(other.size);

			other.texture = 0;
			other.size = { 0, 0 };
		}
		return *this;
	}

	Texture2D::~Texture2D() {
		if (texture != 0)
			glDeleteTextures(1, &texture);
	}

	std::shared_ptr<Texture2D> Texture2D::load(std::filesystem::path path) {
		if (path.empty()) return std::shared_ptr<Texture2D>(nullptr);
		if (path.has_parent_path() && path.parent_path() == "textures")
			path = std::filesystem::path("assets") / path;
		else if (!path.has_parent_path())
			path = std::filesystem::path("assets") / "textures" / path;

		if (!std::filesystem::exists(path)) {
			std::cout << "file '" << path.filename() << "' at '" << path.parent_path() << "' does not exist" << std::endl;
			return std::shared_ptr<Texture2D>(nullptr);
		}

		int w = 0;
		int h = 0;

		uintmax_t size = std::filesystem::file_size(path);
		uint8_t* rawData = new uint8_t[size];
		std::ifstream in(path, std::ios::binary);
		in.read((char*) rawData, size);

		stbi_set_flip_vertically_on_load(true);
		const uint8_t* imgData = stbi_load_from_memory((uint8_t*) rawData, size, &w, &h, nullptr, 4);
		delete[] rawData;

		if (imgData) {
			std::shared_ptr<Texture2D> texture = std::make_shared<Texture2D>(imgData, w, h);
			stbi_image_free((void*) imgData);
			return texture;
		} else {
			std::cerr << "couldn't load image: '" << stbi_failure_reason() << "'" << std::endl;
		}
	}

	void Texture2D::bind() {
		glBindTexture(GL_TEXTURE_2D, texture);
	}

	glm::ivec2 Texture2D::getSize() const {
		return size;
	}
}
