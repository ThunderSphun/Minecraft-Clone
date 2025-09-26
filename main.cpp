#include "shader.h"
#include "shaderProgram.h"

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

	std::shared_ptr<Minecraft::Assets::ShaderProgram> program = Minecraft::Assets::ShaderProgram::create();
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
	glm::mat4x4 view = glm::lookAt(glm::vec3(0, 0, -1), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	glm::mat4x4 proj = glm::perspective(45.0f, 800 / 600.0f, 0.1f, 100.0f);

	glm::mat4x4 mvp = proj * view * model;

	program->setUniform("mvpMatrix", mvp);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	while (!glfwWindowShouldClose(window)) {
		program->update();

		glfwPollEvents();

		double time = glfwGetTime();
		static double pTime = time;

		program->setUniform("time", (float) time);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		glClearColor(bgCol.r, bgCol.g, bgCol.b, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		static glm::vec3 triangleVertices[] = {
			{glm::cos(0.0f * glm::two_pi<float>() / 3), glm::sin(0.0f * glm::two_pi<float>() / 3), 1.0f},
			{glm::cos(1.0f * glm::two_pi<float>() / 3), glm::sin(1.0f * glm::two_pi<float>() / 3), 1.0f},
			{glm::cos(2.0f * glm::two_pi<float>() / 3), glm::sin(2.0f * glm::two_pi<float>() / 3), 1.0f},
		};
		static glm::vec3 triangleColors[] = {
			{1, 0, 0},
			{0, 1, 0},
			{0, 0, 1},
		};
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), triangleVertices);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), triangleColors);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		ImGui::Begin("Debug menu");
		ImGui::ColorEdit3("clear color", glm::value_ptr(bgCol));
		ImGui::Text("frame duration: %f", time - pTime);
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
