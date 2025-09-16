#include "shader.h"
#include "shaderProgram.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_glfw.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

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
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

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
	ImGuiStyle& style = ImGui::GetStyle();

	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
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

	Shader vertShader;
	Shader fragShader;

	try {
		vertShader = Shader("simple", GL_VERTEX_SHADER);
	} catch (const std::runtime_error& ex) {
		std::cerr << ex.what() << std::endl;
		exit(-1);
	}

	try {
		fragShader = Shader("simple", GL_FRAGMENT_SHADER);
	} catch (const std::runtime_error& ex) {
		std::cerr << ex.what() << std::endl;
		exit(-1);
	}

	ShaderProgram* shaderProgram = new ShaderProgram();
	shaderProgram
		->attachShader(vertShader)
		->attachShader(fragShader)
		->bindAttribute(0, "a_position")
		->bindAttribute(1, "a_color")
		->linkProgram()
		->useProgram();

	ImGuiIO& io = ImGui::GetIO();

	glm::vec3 bgCol(0.9, 0.9, 1.0f);

	while (!glfwWindowShouldClose(window)) {
		double time = glfwGetTime();
		static double pTime = time;
		glfwPollEvents();
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		glClearColor(bgCol.r, bgCol.g, bgCol.b, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ImGui::Begin("Debug menu");
		ImGui::ColorEdit3("clear color", glm::value_ptr(bgCol));
		ImGui::Text("frame duration: %f", time - pTime);
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}

		pTime = time;
		glfwSwapBuffers(window);
	}

	delete shaderProgram;
}
