#define GLM_ENABLE_EXPERIMENTAL

#include "shader.h"
#include "renderObject.h"
#include "texture.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_glfw.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/euler_angles.hpp>

#include <cstdlib>
#include <iostream>
#include <format>

GLFWwindow* window = nullptr;
GLFWcursor* cursor = nullptr;

void init() {
	//std::atexit([]() { std::cin.get(); });

	glfwSetErrorCallback([](int error, const char* description) {
		std::cerr << std::format("GLFW Error {}: {}", error, description) << std::endl;
	});

	if (!glfwInit())
		exit(-1);

	std::atexit(glfwTerminate);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(1080, 720, "Minecraft", nullptr, nullptr);
	if (window == nullptr)
		exit(-1);

	std::atexit([]() { glfwDestroyWindow(window); });
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	{
		GLenum err = glewInit();
		if (err != GLEW_OK) {
			std::cerr << "Could not initialize GLEW: " << glewGetErrorString(err) << std::endl;
			exit(-1);
		}
	}

	glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height) {
		glViewport(0, 0, width, height);
	});
	glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mod) {
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
			if (!ImGui::IsAnyItemActive() &&
				// needs to happen in a callback, otherwise the popup is already closed
				!ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel))
				glfwSetWindowShouldClose(window, true);
	});

	cursor = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
	std::atexit([]() { glfwDestroyCursor(cursor); });
	glfwSetCursor(window, cursor);

	if (!IMGUI_CHECKVERSION())
		exit(-1);

	ImGui::CreateContext();
	std::atexit([]() { ImGui::DestroyContext(); });
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	io.ConfigViewportsNoTaskBarIcon = true;

	ImGui::StyleColorsDark();

	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		ImGuiStyle& style = ImGui::GetStyle();
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	if (!ImGui_ImplGlfw_InitForOpenGL(window, true))
		exit(-1);

	std::atexit(ImGui_ImplGlfw_Shutdown);

	if (!ImGui_ImplOpenGL3_Init("#version 460"))
		exit(-1);

	std::atexit(ImGui_ImplOpenGL3_Shutdown);
}

void renderOpenGLConfigMenu() {
	ImGui::Begin("OpenGL modes");
	auto openGLToggle = [](GLenum flag, const char* name) {
		bool isActive = glIsEnabled(flag);

		if (ImGui::Checkbox(name, &isActive))
			if (!isActive) // already toggled, so use inverse
				glDisable(flag);
			else
				glEnable(flag);

		return isActive; // might be toggled already, but is ok because feature is enabled/disabled already then
	};

	if (openGLToggle(GL_DEPTH_TEST, "depth test")) {
		ImGui::Indent();

		static bool clearDepthBuffer = true;
		ImGui::Checkbox("auto clear depth buffer", &clearDepthBuffer);
		if (clearDepthBuffer || ImGui::Button("clear depth buffer"))
			glClear(GL_DEPTH_BUFFER_BIT);

		const char* names[] = { "false", "a < b", "a == b", "a <= b", "a > b", "a != b", "a >= b", "true", };
		GLenum values[] = { GL_NEVER, GL_LESS, GL_EQUAL, GL_LEQUAL, GL_GREATER, GL_NOTEQUAL, GL_GEQUAL, GL_ALWAYS, };

		int value = 0;
		glGetIntegerv(GL_DEPTH_FUNC, &value);
		for (size_t i = 0; i < sizeof(values) / sizeof(values[0]); i++)
			if (value == values[i])
				value = i;

		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x / 2);
		if (ImGui::BeginCombo("depth function", names[value])) {
			for (size_t i = 0; i < sizeof(names) / sizeof(names[0]); i++) {
				const bool is_selected = value == i;
				if (ImGui::Selectable(names[i], is_selected)) {
					value = i;
					glDepthFunc(values[i]);
				}

				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		ImGui::Unindent();
	}

	if (openGLToggle(GL_BLEND, "alpha")) {
		GLenum funcValues[] = {
			GL_ZERO, GL_ONE,
			GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_DST_COLOR, GL_ONE_MINUS_DST_COLOR,
			GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA,
			GL_CONSTANT_COLOR, GL_ONE_MINUS_CONSTANT_COLOR, GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA,
		};
		auto funcCombo = [](const char* name, int& value) {
			const char* names[] = {
				"zero", "one",
				"src_color", "one_minus_src_color", "dst_color", "one_minus_dst_color",
				"src_alpha", "one_minus_src_alpha", "dst_alpha", "one_minus_dst_alpha",
				"constant_color", "one_minus_constant_color", "constant_alpha", "one_minus_constant_alpha",
			};

			bool update = false;

			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x / 2);
			if (ImGui::BeginCombo(name, names[value])) {
				for (size_t i = 0; i < sizeof(names) / sizeof(names[0]); i++) {
					const bool is_selected = value == i;
					if (ImGui::Selectable(names[i], is_selected)) {
						value = i;
						update = true;
					}

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			return update;
		};
		int srcRGB = 0;
		int srcA = 0;
		int dstRGB = 0;
		int dstA = 0;
		glGetIntegerv(GL_BLEND_SRC_RGB, &srcRGB);
		glGetIntegerv(GL_BLEND_SRC_ALPHA, &srcA);
		glGetIntegerv(GL_BLEND_DST_RGB, &dstRGB);
		glGetIntegerv(GL_BLEND_DST_ALPHA, &dstA);
		for (size_t i = 0; i < sizeof(funcValues) / sizeof(funcValues[0]); i++) {
			if (srcRGB == funcValues[i]) srcRGB = i;
			if (srcA   == funcValues[i]) srcA   = i;
			if (dstRGB == funcValues[i]) dstRGB = i;
			if (dstA   == funcValues[i]) dstA   = i;
		}

		GLenum equationValues[] = { GL_FUNC_ADD, GL_FUNC_SUBTRACT, GL_FUNC_REVERSE_SUBTRACT, GL_MIN, GL_MAX, };
		auto equationCombo = [](const char* name, int& value) {
			const char* names[] = { "add", "subtract", "reverse subtract", "min", "max", };

			bool update = false;

			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x / 2);
			if (ImGui::BeginCombo(name, names[value])) {
				for (size_t i = 0; i < sizeof(names) / sizeof(names[0]); i++) {
					const bool is_selected = value == i;
					if (ImGui::Selectable(names[i], is_selected)) {
						value = i;
						update = true;
					}

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			return update;
		};
		int equationRGB = 0;
		int equationA = 0;
		glGetIntegerv(GL_BLEND_EQUATION_RGB, &equationRGB);
		glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &equationA);
		for (size_t i = 0; i < sizeof(equationValues) / sizeof(equationValues[0]); i++) {
			if (equationRGB == equationValues[i]) equationRGB = i;
			if (equationA   == equationValues[i]) equationA   = i;
		}

		static bool useSeperate = false;
		bool funcUpdate = false;
		bool equationUpdate = false;

		ImGui::Indent();
		ImGui::Checkbox("use seperate", &useSeperate);

		funcUpdate |= funcCombo("blend source", srcRGB);
		if (useSeperate) funcUpdate |= funcCombo("blend source alpha", srcA);
		funcUpdate |= funcCombo("blend dest", dstRGB);
		if (useSeperate) funcUpdate |= funcCombo("blend dest alpha", dstA);

		if (srcRGB >= 10 || srcA >= 10 || dstRGB >= 10 || dstA >= 10) {
			glm::vec4 color(0, 0, 0, 1);
			glGetFloatv(GL_BLEND_COLOR, glm::value_ptr(color));

			if (ImGui::ColorEdit4("blend color", glm::value_ptr(color)))
				glBlendColor(color.r, color.g, color.b, color.a);
		}

		equationUpdate |= equationCombo("blend equation", equationRGB);
		if (useSeperate) equationUpdate |= equationCombo("blend equation alpha", equationA);
		ImGui::Unindent();

		if (funcUpdate) {
			if (useSeperate)
				glBlendFuncSeparate(funcValues[srcRGB], funcValues[dstRGB], funcValues[srcA], funcValues[dstA]);
			else
				glBlendFunc(funcValues[srcRGB], funcValues[dstRGB]);
		}
		if (equationUpdate) {
			if (useSeperate)
				glBlendEquationSeparate(equationValues[equationRGB], equationValues[equationA]);
			else
				glBlendEquation(equationValues[equationRGB]);
		}
	}

	if (openGLToggle(GL_CULL_FACE, "cull face")) {
		{
			const char* names[] = { "front", "back", };
			GLenum values[] = { GL_FRONT, GL_BACK, };
			static int value = 1;
			bool update = false;

			ImGui::Indent();
			for (size_t i = 0; i < sizeof(names) / sizeof(names[0]); i++) {
				if (i > 0)
					ImGui::SameLine();
				update |= ImGui::RadioButton(names[i], &value, i);
			}
			ImGui::Unindent();

			if (update)
				glCullFace(values[value]);
		}
		{
			const char* names[] = { "clockwise", "counterclockwise", };
			GLenum values[] = { GL_CW, GL_CCW, };
			static int value = 1;
			bool update = false;

			ImGui::Indent();
			for (size_t i = 0; i < sizeof(names) / sizeof(names[0]); i++) {
				if (i > 0)
					ImGui::SameLine();
				update |= ImGui::RadioButton(names[i], &value, i);
			}
			ImGui::Unindent();

			if (update)
				glFrontFace(values[value]);
		}
	}

	{
		const char* names[] = { "point", "line", "fill", };
		GLenum values[] = { GL_POINT, GL_LINE, GL_FILL, };
		static int value = 2;
		bool update = false;

		for (size_t i = 0; i < sizeof(names) / sizeof(names[0]); i++) {
			if (i > 0)
				ImGui::SameLine();
			update |= ImGui::RadioButton(names[i], &value, i);
		}

		if (update)
			glPolygonMode(GL_FRONT_AND_BACK, values[value]);

		ImGui::Indent();
		switch (values[value]) {
		case GL_POINT: {
			float range[] = { 1, 1 };
			float currentSize = 1;

			glGetFloatv(GL_POINT_SIZE_RANGE, range);
			glGetFloatv(GL_POINT_SIZE, &currentSize);

			if (ImGui::SliderFloat("Point size", &currentSize, range[0], range[1], nullptr, ImGuiSliderFlags_Logarithmic | ImGuiSliderFlags_AlwaysClamp))
				glPointSize(glm::clamp(currentSize, range[0], range[1]));
		} break;
		case GL_LINE: {
			bool isSmooth = glIsEnabled(GL_LINE_SMOOTH);
			float range[] = { 1, 1 };
			float currentSize = 1;

			glGetFloatv(isSmooth ? GL_SMOOTH_LINE_WIDTH_RANGE : GL_ALIASED_LINE_WIDTH_RANGE, range);
			glGetFloatv(GL_LINE_WIDTH, &currentSize);

			bool isChanged = ImGui::SliderFloat("line width", &currentSize, range[0], range[1], nullptr, ImGuiSliderFlags_AlwaysClamp);
			isChanged |= openGLToggle(GL_LINE_SMOOTH, "use smooth lines") == isSmooth;

			if (isChanged)
				glLineWidth(glm::clamp(currentSize, range[0], range[1]));

		} break;
		}
		ImGui::Unindent();
	}

	ImGui::End();
}

Minecraft::Assets::VAO createCube(uint8_t atlasIndex) {
	static const glm::vec3 vertices[] = {
		// back
		{-0.5f, -0.5f, -0.5f},
		{-0.5f, +0.5f, -0.5f},
		{+0.5f, -0.5f, -0.5f},
		{+0.5f, +0.5f, -0.5f},
		// front
		{-0.5f, -0.5f, +0.5f},
		{-0.5f, +0.5f, +0.5f},
		{+0.5f, -0.5f, +0.5f},
		{+0.5f, +0.5f, +0.5f},

		// top
		{-0.5f, +0.5f, -0.5f},
		{-0.5f, +0.5f, +0.5f},
		{+0.5f, +0.5f, -0.5f},
		{+0.5f, +0.5f, +0.5f},
		// bottom
		{-0.5f, -0.5f, -0.5f},
		{-0.5f, -0.5f, +0.5f},
		{+0.5f, -0.5f, -0.5f},
		{+0.5f, -0.5f, +0.5f},

		// right
		{+0.5f, -0.5f, -0.5f},
		{+0.5f, -0.5f, +0.5f},
		{+0.5f, +0.5f, -0.5f},
		{+0.5f, +0.5f, +0.5f},
		// left
		{-0.5f, -0.5f, -0.5f},
		{-0.5f, -0.5f, +0.5f},
		{-0.5f, +0.5f, -0.5f},
		{-0.5f, +0.5f, +0.5f},
	};
	static const glm::vec4 colors[] = {
		// back
		{1, 0, 0, 1},
		{0, 1, 0, 1},
		{0, 0, 1, 1},
		{0, 0, 0, 1},
		// front
		{0, 1, 1, 1},
		{1, 0, 1, 1},
		{1, 1, 0, 1},
		{1, 1, 1, 1},

		// top
		{0, 1, 0, 1},
		{1, 0, 1, 1},
		{0, 0, 0, 1},
		{1, 1, 1, 1},
		// bottom
		{1, 0, 0, 1},
		{0, 1, 1, 1},
		{0, 0, 1, 1},
		{1, 1, 0, 1},

		// right
		{0, 0, 1, 1},
		{1, 1, 0, 1},
		{0, 0, 0, 1},
		{1, 1, 1, 1},
		// left
		{1, 0, 0, 1},
		{0, 1, 1, 1},
		{0, 1, 0, 1},
		{1, 0, 1, 1},
	};
	static const uint32_t indices[] = {
		 0,  2,  1,   2,  3,  1, // Back
		 4,  5,  6,   6,  5,  7, // Front

		 8, 10,  9,  10, 11,  9, // Top
		12, 13, 14,  14, 13, 15, // Bottom

		16, 17, 18,  17, 19, 18, // Right
		20, 22, 21,  21, 22, 23, // Left
	};
	static const glm::vec2 spriteSize = {1 / 16.0, 1 / 16.0};

	glm::vec2 uvCorner = {spriteSize.x * (atlasIndex % 16), spriteSize.y * (atlasIndex / 16)};

	glm::vec2 texcoords[] = {
		// back
		uvCorner + spriteSize * glm::vec2{0, 0},
		uvCorner + spriteSize * glm::vec2{0, 1},
		uvCorner + spriteSize * glm::vec2{1, 0},
		uvCorner + spriteSize * glm::vec2{1, 1},
		// front
		uvCorner + spriteSize * glm::vec2{1, 0},
		uvCorner + spriteSize * glm::vec2{1, 1},
		uvCorner + spriteSize * glm::vec2{0, 0},
		uvCorner + spriteSize * glm::vec2{0, 1},

		// top
		uvCorner + spriteSize * glm::vec2{1, 1},
		uvCorner + spriteSize * glm::vec2{1, 0},
		uvCorner + spriteSize * glm::vec2{0, 1},
		uvCorner + spriteSize * glm::vec2{0, 0},
		// bottom
		uvCorner + spriteSize * glm::vec2{1, 1},
		uvCorner + spriteSize * glm::vec2{1, 0},
		uvCorner + spriteSize * glm::vec2{0, 1},
		uvCorner + spriteSize * glm::vec2{0, 0},

		// right
		uvCorner + spriteSize * glm::vec2{0, 0},
		uvCorner + spriteSize * glm::vec2{1, 0},
		uvCorner + spriteSize * glm::vec2{0, 1},
		uvCorner + spriteSize * glm::vec2{1, 1},
		// left
		uvCorner + spriteSize * glm::vec2{1, 0},
		uvCorner + spriteSize * glm::vec2{0, 0},
		uvCorner + spriteSize * glm::vec2{1, 1},
		uvCorner + spriteSize * glm::vec2{0, 1},
	};

	return Minecraft::Assets::VAO::create(
		[texcoords]() {
			return Minecraft::Assets::VBO::create([texcoords](GLuint vbo) {
				glNamedBufferData(vbo, sizeof(vertices) + sizeof(colors) + sizeof(texcoords), nullptr, GL_STATIC_DRAW);

				glNamedBufferSubData(vbo, 0, sizeof(vertices), vertices);
				glNamedBufferSubData(vbo, sizeof(vertices), sizeof(colors), colors);
				glNamedBufferSubData(vbo, sizeof(vertices) + sizeof(colors), sizeof(texcoords), texcoords);

				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
				glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (GLvoid*) sizeof(vertices));
				glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (GLvoid*) (sizeof(vertices) + sizeof(colors)));

				glEnableVertexAttribArray(0);
				glEnableVertexAttribArray(1);
				glEnableVertexAttribArray(2);

				return sizeof(vertices) / sizeof(vertices[0]);
			});
		},
		[]() {
			return Minecraft::Assets::EBO::create([](GLuint ebo) {
				glNamedBufferData(ebo, sizeof(indices), indices, GL_STATIC_DRAW);

				return sizeof(indices) / sizeof(indices[0]);
			});
		}
	);
}

int main() {
	init();

	std::shared_ptr<Minecraft::Assets::Shader::Program> program = Minecraft::Assets::Shader::Program::create();
	program
		->attachShader(Minecraft::Assets::Shader::parse(std::filesystem::path("simple"), GL_VERTEX_SHADER))
		->attachShader(Minecraft::Assets::Shader::parse(std::filesystem::path("simple"), GL_FRAGMENT_SHADER))
		->bindAttribute(0, "a_position")
		->bindAttribute(1, "a_color")
		->bindAttribute(2, "a_texcoord")
		->link()
		->use();

	std::shared_ptr<Minecraft::Assets::Texture2D> img = Minecraft::Assets::Texture2D::load(std::filesystem::path("blocks.png"));
	img->bind();

	ImGuiIO& io = ImGui::GetIO();

	glm::vec3 bgCol(0.9, 0.9, 1.0f);

	glm::mat4 model = glm::mat4(1);
	glm::mat4 view = glm::mat4(1);
	glm::mat4 proj = glm::perspective(45.0f, 1080 / 720.0f, 0.1f, 100.0f);

	program->setUniform("modelMatrix", model);
	program->setUniform("viewMatrix", view);
	program->setUniform("projectionMatrix", proj);

	Minecraft::Assets::VAO cube1 = createCube(0);
	Minecraft::Assets::VAO cube2 = createCube(1);

	glEnable(GL_DEPTH_TEST);

	glDisable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	while (!glfwWindowShouldClose(window)) {
		if (program->update()) {
			program->setUniform("modelMatrix", model);
			program->setUniform("viewMatrix", view);
			program->setUniform("projectionMatrix", proj);
		}

		glfwPollEvents();

		double time = glfwGetTime();
		static double pTime = time;

		program->setUniform("time", (float) time);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		glClearColor(bgCol.r, bgCol.g, bgCol.b, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		ImGui::Begin("Debug menu");
		ImGui::Text("frame duration: %f", time - pTime);
		ImGui::ColorEdit3("clear color", glm::value_ptr(bgCol));
		ImGui::Separator();
		{
			static bool renderCube1 = true;
			static bool renderCube2 = true;

			ImGui::Checkbox("Render cube 1", &renderCube1);
			ImGui::Checkbox("Render cube 2", &renderCube2);

			if (renderCube1) {
				model = glm::mat4(1);
				if (renderCube2)
					model = glm::translate(model, { -0.5, 0, 0 });

				program->setUniform("modelMatrix", model);

				cube1.draw();
			}

			if (renderCube2) {
				model = glm::mat4(1);
				if (renderCube1)
					model = glm::translate(model, { 0.5, 0, 0 });

				program->setUniform("modelMatrix", model);
				cube2.draw();
			}
		}
		static float pitch = 0;
		static float yaw = 0;
		static float roll = 0;
		static bool changedAngle = true;
		ImGui::BeginGroup();
		changedAngle |= ImGui::SliderAngle("pitch", &pitch, -90, 90);
		changedAngle |= ImGui::SliderAngle("Yaw", &yaw);
		changedAngle |= ImGui::SliderAngle("Roll", &roll);
		ImGui::EndGroup();
		ImGui::SameLine();
		ImGui::BeginGroup();
		ImGui::BeginDisabled(glm::abs(pitch) < 0.01);
		if (ImGui::Button("reset##pitch", { -FLT_MIN, 0 })) {
			pitch = 0;
			changedAngle = true;
		}
		ImGui::EndDisabled();
		ImGui::BeginDisabled(glm::abs(yaw) < 0.01);
		if (ImGui::Button("reset##yaw", { -FLT_MIN, 0 })) {
			yaw = 0;
			changedAngle = true;
		}
		ImGui::EndDisabled();
		ImGui::BeginDisabled(glm::abs(roll) < 0.01);
		if (ImGui::Button("reset##roll", { -FLT_MIN, 0 })) {
			roll = 0;
			changedAngle = true;
		}
		ImGui::EndDisabled();
		ImGui::EndGroup();
		ImGui::BeginDisabled(glm::abs(pitch) < 0.01 && glm::abs(yaw) < 0.01 && glm::abs(roll) < 0.01);
		if (ImGui::Button("reset", { -FLT_MIN, 0 })) {
			pitch = 0;
			yaw = 0;
			roll = 0;
			changedAngle = true;
		}
		ImGui::EndDisabled();
		if (changedAngle) {
			glm::mat4 rotation = glm::yawPitchRoll(yaw, pitch, roll);

			view = glm::lookAt(glm::vec3(rotation * glm::vec4(0, 0, 3, 0)), glm::vec3(0), glm::vec3(rotation * glm::vec4(0, 1, 0, 0)));

			program->setUniform("viewMatrix", view);

			changedAngle = false;
		}
		ImGui::End();

		renderOpenGLConfigMenu();

		if (ImGui::Begin("textures")) {
			static glm::ivec2 uv{0, 0};
			ImGui::Text("block texture atlas");
			ImVec2 screenPos = ImGui::GetCursorScreenPos();
			ImVec2 pos = ImGui::GetCursorPos();
			ImGui::Image(img->getId(), ImVec2(img->getSize().x, img->getSize().y));
			if (ImGui::IsItemHovered())
				ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
			ImGui::SetCursorPos(pos);
			ImGui::InvisibleButton("block texture atlas", ImVec2(img->getSize().x, img->getSize().y));
			if (ImGui::IsItemActive()) {
				glm::vec2 clickedPos{io.MousePos.x - screenPos.x, io.MousePos.y - screenPos.y};
				clickedPos /= 16;
				if (
					clickedPos.x >= 0 && clickedPos.x < 16 &&
					clickedPos.y >= 0 && clickedPos.y < 16
				) {
					io.ConfigWindowsMoveFromTitleBarOnly = true;
					uv = (glm::ivec2) clickedPos;
				}
			} else if (io.ConfigWindowsMoveFromTitleBarOnly)
				io.ConfigWindowsMoveFromTitleBarOnly = false;
			ImGui::SliderInt2("uvs", glm::value_ptr(uv), 0, 15, nullptr, ImGuiSliderFlags_AlwaysClamp);
			ImGui::Image(img->getId(), {128, 128}, {uv.x / 16.0f, uv.y / 16.0f}, {(uv.x + 1) / 16.0f, (uv.y + 1) / 16.0f});
		}
		ImGui::End();

		ImGui::ShowDemoWindow();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}

		pTime = time;
		glfwSwapBuffers(window);
	}
}
