#define GLM_ENABLE_EXPERIMENTAL

#include "shader.h"
#include "renderObject.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_glfw.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <cstdlib>
#include <iostream>
#include <format>

GLFWwindow* window = nullptr;
GLFWcursor* cursor = nullptr;

void init() {
	glfwSetErrorCallback([](int error, const char* description) {
		std::cerr << std::format("GLFW Error {}: {}", error, description) << std::endl;
	});

	if (!glfwInit())
		exit(-1);

	std::atexit(glfwTerminate);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

	window = glfwCreateWindow(1080, 720, "Minecraft", nullptr, nullptr);
	if (window == nullptr)
		exit(-1);

	std::atexit([](){glfwDestroyWindow(window);});
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

	cursor = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
	std::atexit([](){glfwDestroyCursor(cursor);});
	glfwSetCursor(window, cursor);

	if (!IMGUI_CHECKVERSION())
		exit(-1);

	ImGui::CreateContext();
	std::atexit([](){ImGui::DestroyContext();});
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

	if (!ImGui_ImplOpenGL3_Init("#version 430"))
		exit(-1);

	std::atexit(ImGui_ImplOpenGL3_Shutdown);
}

int main() {
	init();

	std::shared_ptr<Minecraft::Assets::Shader::Program> program = Minecraft::Assets::Shader::Program::create();
	program
		->attachShader(Minecraft::Assets::Shader::parse(std::filesystem::path("simple"), GL_VERTEX_SHADER))
		->attachShader(Minecraft::Assets::Shader::parse(std::filesystem::path("simple"), GL_FRAGMENT_SHADER))
		->bindAttribute(0, "a_position")
		->bindAttribute(1, "a_color")
		->link()
		->use();

	ImGuiIO& io = ImGui::GetIO();

	glm::vec3 bgCol(0.9, 0.9, 1.0f);

	glm::mat4x4 model = glm::mat4x4(1);
	glm::mat4x4 view = glm::lookAt(glm::vec3(0, 1.666, -5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	glm::mat4x4 proj = glm::perspective(45.0f, 1080 / 720.0f, 0.1f, 100.0f);

	glm::mat4x4 mvp = proj * view * model;

	glm::vec3 vertices[] = {
		{-0.5f, -0.5f, -0.5f},
		{-0.5f, +0.5f, -0.5f},
		{+0.5f, -0.5f, -0.5f},
		{+0.5f, +0.5f, -0.5f},
		{-0.5f, -0.5f, +0.5f},
		{-0.5f, +0.5f, +0.5f},
		{+0.5f, -0.5f, +0.5f},
		{+0.5f, +0.5f, +0.5f},
	};
	glm::vec4 colors[] = {
		{1, 0, 0, 1},
		{0, 1, 0, 1},
		{0, 0, 1, 1},
		{0, 0, 0, 1},
		{0, 1, 1, 1},
		{1, 0, 1, 1},
		{1, 1, 0, 1},
		{1, 1, 1, 1},
	};
	uint32_t indices[] = {
		0, 1, 2,  1, 2, 3, // back
		4, 5, 6,  5, 6, 7, // front
		0, 1, 4,  1, 4, 5, // left
		2, 3, 6,  3, 6, 7, // right
		0, 2, 4,  2, 4, 6, // bottom
		1, 3, 5,  3, 5, 7, // top
	};

	Minecraft::Assets::RenderObject cube = Minecraft::Assets::RenderObject::create([vertices, colors](GLuint vbo){
		glNamedBufferData(vbo, sizeof(vertices) + sizeof(colors), nullptr, GL_STATIC_DRAW);
		glNamedBufferSubData(vbo, 0, sizeof(vertices), vertices);
		glNamedBufferSubData(vbo, sizeof(vertices), sizeof(colors), colors);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (GLvoid*) sizeof(vertices));
		glEnableVertexAttribArray(1);

		return 36;
	}, [indices](GLuint ebo){
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	});

	glEnable(GL_DEPTH_TEST);

	while (!glfwWindowShouldClose(window)) {
		program->update();
		program->setUniform("mvpMatrix", mvp);

		glfwPollEvents();

		double time = glfwGetTime();
		static double pTime = time;

		program->setUniform("time", (float) time);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		glClearColor(bgCol.r, bgCol.g, bgCol.b, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		cube.draw();

		ImGui::Begin("Debug menu");
		ImGui::ColorEdit3("clear color", glm::value_ptr(bgCol));
		ImGui::Text("frame duration: %f", time - pTime);
		static float pitch = 0;
		static float yaw = 0;
		bool changedAngle = false;
		changedAngle |= ImGui::SliderAngle("pitch", &pitch, -90, 90);
		ImGui::SameLine();
		if (ImGui::Button("reset##pitch")) {
			pitch = 0;
			changedAngle = true;
		}
		changedAngle |= ImGui::SliderAngle("Yaw", &yaw);
		ImGui::SameLine();
		if (ImGui::Button("reset##yaw")) {
			yaw = 0;
			changedAngle = true;
		}
		if (changedAngle) {
			float x = 5.0f * cos(pitch) * sin(yaw);
			float y = 5.0f * sin(pitch);
			float z = 5.0f * cos(pitch) * cos(yaw);

			glm::mat4 view = glm::lookAt({ x, y, z }, glm::vec3(0), { 0.0f, 1.0f, 0.0f });
			mvp = proj * view * model;
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
