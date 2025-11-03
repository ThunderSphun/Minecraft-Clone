#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <filesystem>
#include <memory>

namespace Minecraft::Assets {
	// TODO think about making 'GLuint texture' a shared_ptr instead, to get rid of needing texture2D to be a shared_ptr
	class Texture2D {
	public:
		Texture2D(const Texture2D&) = delete;
		Texture2D(Texture2D&& other) noexcept;
		Texture2D& operator=(const Texture2D&) = delete;
		Texture2D& operator=(Texture2D&& other) noexcept;
		~Texture2D();

		[[nodiscard]] static std::shared_ptr<Texture2D> load(std::filesystem::path path);

		void bind();

		glm::ivec2 getSize() const;
		GLuint getId() const;

		Texture2D(const uint8_t* data, int width, int height);
	private:

		GLuint texture = 0;

		glm::ivec2 size = { 0, 0 };
	};
}
