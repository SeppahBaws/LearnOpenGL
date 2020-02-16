// TODO: continue from here: https://learnopengl.com/Advanced-OpenGL/Depth-testing

// ReSharper disable once CppUnusedIncludeDirective
#include <vld.h>

#include "deps/imgui/imgui.h"
#include "deps/imgui/imgui_impl_glfw.h"
#include "deps/imgui/imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <array>

#include "helpers/Logger.h"

#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "stb_image.h"
#include "Texture.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void error_callback(int error, const char* description);
void processInput(GLFWwindow* window);

void CenterWindow(GLFWwindow* window, GLFWmonitor* monitor);

int screenWidth = 1280;
int screenHeight = 720;

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = screenWidth / 2.0f;
float lastY = screenHeight / 2.0f;
bool firstMouse = true;

bool gameModeKeyDown = false;
bool flashLightOn = false;
bool flashLightKeyDown = false;

float deltaTime = 0.0f; // Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

int main()
{
	Logger::Init();
	
	GLFWwindow* window;

	// Initialize GLFW
	if (!glfwInit())
	{
		Logger::LogError("Failed to initialize GLFW!");
		return -1;
	}

	glfwSetErrorCallback(error_callback);

	const char* glsl_version = "#version 450"; // ImGui GLSL version (CHANGE THIS WHEN CHANGING THE GLFW CONTEXT VERSION!!!!)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create a windowed mode window and its OpenGL context
	window = glfwCreateWindow(screenWidth, screenHeight, "Hello World", nullptr, nullptr);
	if (!window)
	{
		Logger::LogError("Failed to create GLFW window!");
		glfwTerminate();
		return -1;
	}

	// Center the window
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	CenterWindow(window, monitor);

	// Make the window's context current
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Initialize GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		Logger::LogError("Failed to initialize GLAD!");
		glfwTerminate();
		return -1;
	}

	// Initialize ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	// Disable ImGui mouse by default
	io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
	(void)io;

	// Setup ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	glEnable(GL_DEPTH_TEST);

	std::stringstream ss;
	ss << "--------------------------------\n";
	ss << " OpenGL Info:\n";
	ss << "   Vendor: " << glGetString(GL_VENDOR) << "\n";
	ss << "   Renderer: " << glGetString(GL_RENDERER) << "\n";
	ss << "   Version: " << glGetString(GL_VERSION) << "\n";
	ss << "--------------------------------";
	Logger::LogSuccess(ss.str());

	float boxVertices[] = {
		// positions
		-0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,

		-0.5f, -0.5f,  0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,

		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,

		 0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,

		-0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f, -0.5f,

		-0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f,
	};

	// Box
	unsigned int boxVAO, boxVBO;
	glGenVertexArrays(1, &boxVAO);
	glGenBuffers(1, &boxVBO);

	glBindBuffer(GL_ARRAY_BUFFER, boxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(boxVertices), boxVertices, GL_STATIC_DRAW);

	glBindVertexArray(boxVAO);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);


	Shader shaderRed("res/shaders/ubo_test.vs", "res/shaders/ubo_test_red.fs");
	Shader shaderGreen("res/shaders/ubo_test.vs", "res/shaders/ubo_test_green.fs");
	Shader shaderBlue("res/shaders/ubo_test.vs", "res/shaders/ubo_test_blue.fs");
	Shader shaderYellow("res/shaders/ubo_test.vs", "res/shaders/ubo_test_yellow.fs");


	unsigned int uniformBlockIndexRed		= glGetUniformBlockIndex(shaderRed.GetId(), "Matrices");
	unsigned int uniformBlockIndexGreen		= glGetUniformBlockIndex(shaderGreen.GetId(), "Matrices");
	unsigned int uniformBlockIndexBlue		= glGetUniformBlockIndex(shaderBlue.GetId(), "Matrices");
	unsigned int uniformBlockIndexYellow	= glGetUniformBlockIndex(shaderYellow.GetId(), "Matrices");

	glUniformBlockBinding(shaderRed.GetId(), uniformBlockIndexRed, 0);
	glUniformBlockBinding(shaderGreen.GetId(), uniformBlockIndexGreen, 0);
	glUniformBlockBinding(shaderBlue.GetId(), uniformBlockIndexBlue, 0);
	glUniformBlockBinding(shaderYellow.GetId(), uniformBlockIndexYellow, 0);

	unsigned int uboMatrices;
	glGenBuffers(1, &uboMatrices);

	glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);	// Allocate enough data for the uniform buffer
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboMatrices, 0, 2 * sizeof(glm::mat4));

	// Loop until the user closes the window
	while (!glfwWindowShouldClose(window))
	{
		// DeltaTime calculation
		float currentFrame = float(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Input
		processInput(window);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// ==============================================================
		// Rendering Preparation
		// ==============================================================
		// Now we only have to set our view and projection matrices once per frame.
		glm::mat4 projection = glm::perspective(glm::radians(camera.GetZoom()), (float)screenWidth / (float)screenHeight, 0.1f, 1000.0f);
		glm::mat4 view = camera.GetViewMatrix();
		glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		// ==============================================================
		// Rendering
		// ==============================================================

		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 model = glm::mat4(1.0f);

		// Red box
		glBindVertexArray(boxVAO);
		model = glm::translate(glm::mat4(1.0f), glm::vec3(-0.75f, 0.75f, 0.0f)); // move top-left
		shaderRed.Use();
		shaderRed.SetMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// Green box
		glBindVertexArray(boxVAO);
		model = glm::translate(glm::mat4(1.0f), glm::vec3(0.75f, 0.75f, 0.0f)); // move top-right
		shaderGreen.Use();
		shaderGreen.SetMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		
		// Blue box
		glBindVertexArray(boxVAO);
		model = glm::translate(glm::mat4(1.0f), glm::vec3(-0.75f, -0.75f, 0.0f)); // move bottom-left
		shaderBlue.Use();
		shaderBlue.SetMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		
		// Yellow box
		glBindVertexArray(boxVAO);
		model = glm::translate(glm::mat4(1.0f), glm::vec3(0.75f, -0.75f, 0.0f)); // move bottom-right
		shaderYellow.Use();
		shaderYellow.SetMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// ==============================================================
		// End Rendering
		// ==============================================================
		


		// Draw ImGui at the end
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Swap buffers and poll IO events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Cleanup ImGui
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	
	// Cleanup OpenGL buffers
	glDeleteVertexArrays(1, &boxVAO);
	glDeleteBuffers(1, &boxVBO);

	glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	screenWidth = width;
	screenHeight = height;
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = float(xpos);
		lastY = float(ypos);
		firstMouse = false;
	}

	float xoffset = float(xpos) - lastX;
	float yoffset = lastY - float(ypos);
	lastX = float(xpos);
	lastY = float(ypos);

	if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
	{
		camera.ProcessMouseMovement(xoffset, yoffset);
	}
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
	{
		camera.ProcessMouseScroll(float(yoffset));
	}
}

void error_callback(int error, const char* description)
{
	std::cerr << "Error [" << error << "]: " << description << std::endl;
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}

	if (glfwGetKey(window, GLFW_KEY_GRAVE_ACCENT) == GLFW_PRESS)
		gameModeKeyDown = true;

	if (glfwGetKey(window, GLFW_KEY_GRAVE_ACCENT) == GLFW_RELEASE && gameModeKeyDown)
	{
		gameModeKeyDown = false;
		if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
		{
			// Re-enable the mouse
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			ImGuiIO& io = ImGui::GetIO();
			io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
		}
		else
		{
			// Disable the mouse
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			ImGuiIO& io = ImGui::GetIO();
			io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
		}

		// Test with bitfields: https://repl.it/@seppedek/Bitfields
	}

	if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
	{
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			camera.ProcessKeyboard(CameraMovement::FORWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			camera.ProcessKeyboard(CameraMovement::BACKWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			camera.ProcessKeyboard(CameraMovement::LEFT, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			camera.ProcessKeyboard(CameraMovement::RIGHT, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
			camera.ProcessKeyboard(CameraMovement::UP, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
			camera.ProcessKeyboard(CameraMovement::DOWN, deltaTime);

		if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && !flashLightKeyDown)
		{
			flashLightKeyDown = true;
			flashLightOn = !flashLightOn;
		}
		if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE && flashLightKeyDown)
		{
			flashLightKeyDown = false;
		}
	}
}

void CenterWindow(GLFWwindow* window, GLFWmonitor* monitor)
{
	if (!monitor)
		return;

	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	if (!mode)
		return;

	int monitorX, monitorY;
	glfwGetMonitorPos(monitor, &monitorX, &monitorY);

	int windowWidth, windowHeight;
	glfwGetWindowSize(window, &windowWidth, &windowHeight);

	glfwSetWindowPos(window, monitorX + (mode->width - windowWidth) / 2, monitorY + (mode->height - windowHeight) / 2);
}