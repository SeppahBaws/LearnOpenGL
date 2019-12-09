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

#include "helpers/Logger.h"

#include "Shader.h"
#include "Camera.h"
#include "Model.h"

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

	float vertices[] = {
		// positions          // normals           // texture coords
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
	};

	glm::vec3 cubePositions[] = {
		glm::vec3( 0.0f,  0.0f,  0.0f),
		glm::vec3( 2.0f,  5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3( 2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -7.5f),
		glm::vec3( 1.3f, -2.0f, -2.5f),
		glm::vec3( 1.5f,  2.0f, -2.5f),
		glm::vec3( 1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f)
	};

	glm::vec3 pointLightPositions[] = {
		glm::vec3(0.7f,  0.2f,  2.0f),
		glm::vec3(2.3f, -3.3f, -4.0f),
		glm::vec3(-4.0f,  2.0f, -12.0f),
		glm::vec3(0.0f,  0.0f, -3.0f)
	};

	// Framebuffer
	unsigned int fbo;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	// Render texture
	unsigned int texColorBuffer;
	glGenTextures(1, &texColorBuffer);
	glBindTexture(GL_TEXTURE_2D, texColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// attach it to currently bound framebuffer object
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorBuffer, 0);

	// Render Buffer
	unsigned int rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, screenWidth, screenHeight);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		Logger::LogError("ERROR::FRAMEBUFFER:: Framebuffer is not complete!");
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	Shader screenShader("res/shaders/framebuffer_vertex.glsl", "res/shaders/framebuffer_fragment.glsl");

	float quadVertices[] = {
		// positions   // texCoords
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f
	};
	
	unsigned int quadVAO, quadVBO;
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);

	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
	
	glBindVertexArray(quadVAO);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);


	unsigned int cubeVAO, VBO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindVertexArray(cubeVAO);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);



	unsigned int lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);


	// Our custom model that we want to draw
	Model nanosuit("res/models/nanosuit/nanosuit.obj");
	
	// Create shader
	Shader lightingShader("res/shaders/lit_vertex.glsl", "res/shaders/lit_fragment.glsl");
	Shader lampShader("res/shaders/lit_vertex.glsl", "res/shaders/light_fragment.glsl");

	lightingShader.Use();
	lightingShader.SetInt("material.diffuse", 0);
	lightingShader.SetInt("material.specular", 1);

	glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

	// ImGui variables
	glm::vec3 clearColor(0.2f, 0.2f, 0.2f);

	glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f); // decrease the influence
	glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f); // low influence

	glm::vec3 matDiffuse(1.0f, 0.5f, 0.31f);
	glm::vec3 matSpecular(0.5f, 0.5f, 0.5f);
	float matShininess = 32.0f;

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
		// Rendering
		// ==============================================================

		// +================+
		// |   first pass   |
		// +================+
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glClearColor(clearColor.r, clearColor.g, clearColor.b, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

		// Light settings
		lightingShader.Use();
		lightingShader.SetVec3("viewPos", camera.GetPosition());
		lightingShader.SetFloat("material.shininess", matShininess);

		// directional light
		lightingShader.SetVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
		lightingShader.SetVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
		lightingShader.SetVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
		lightingShader.SetVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);

		// point light 1
		lightingShader.SetVec3("pointLights[0].position", pointLightPositions[0]);
		lightingShader.SetVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
		lightingShader.SetVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
		lightingShader.SetVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
		lightingShader.SetFloat("pointLights[0].constant", 1.0f);
		lightingShader.SetFloat("pointLights[0].linear", 0.09);
		lightingShader.SetFloat("pointLights[0].quadratic", 0.032);
		// point light 2
		lightingShader.SetVec3("pointLights[1].position", pointLightPositions[1]);
		lightingShader.SetVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
		lightingShader.SetVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
		lightingShader.SetVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
		lightingShader.SetFloat("pointLights[1].constant", 1.0f);
		lightingShader.SetFloat("pointLights[1].linear", 0.09);
		lightingShader.SetFloat("pointLights[1].quadratic", 0.032);
		// point light 3
		lightingShader.SetVec3("pointLights[2].position", pointLightPositions[2]);
		lightingShader.SetVec3("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
		lightingShader.SetVec3("pointLights[2].diffuse", 0.8f, 0.8f, 0.8f);
		lightingShader.SetVec3("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
		lightingShader.SetFloat("pointLights[2].constant", 1.0f);
		lightingShader.SetFloat("pointLights[2].linear", 0.09);
		lightingShader.SetFloat("pointLights[2].quadratic", 0.032);
		// point light 4
		lightingShader.SetVec3("pointLights[3].position", pointLightPositions[3]);
		lightingShader.SetVec3("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
		lightingShader.SetVec3("pointLights[3].diffuse", 0.8f, 0.8f, 0.8f);
		lightingShader.SetVec3("pointLights[3].specular", 1.0f, 1.0f, 1.0f);
		lightingShader.SetFloat("pointLights[3].constant", 1.0f);
		lightingShader.SetFloat("pointLights[3].linear", 0.09);
		lightingShader.SetFloat("pointLights[3].quadratic", 0.032);
		// spotLight
		lightingShader.SetBool("spotLight.useSpotlight", flashLightOn);
		lightingShader.SetVec3("spotLight.position", camera.GetPosition());
		lightingShader.SetVec3("spotLight.direction", camera.GetDirection());
		lightingShader.SetVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
		lightingShader.SetVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
		lightingShader.SetVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
		lightingShader.SetFloat("spotLight.constant", 1.0f);
		lightingShader.SetFloat("spotLight.linear", 0.09);
		lightingShader.SetFloat("spotLight.quadratic", 0.032);
		lightingShader.SetFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
		lightingShader.SetFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));


		glm::mat4 projection = glm::perspective(glm::radians(camera.GetZoom()), float(screenWidth) / float(screenHeight), 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		lightingShader.SetMat4("projection", projection);
		lightingShader.SetMat4("view", view);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));
		lightingShader.SetMat4("model", model);

		nanosuit.Draw(lightingShader);
		
		// render lamp object
		lampShader.Use();
		lampShader.SetMat4("projection", projection);
		lampShader.SetMat4("view", view);
		
		glBindVertexArray(lightVAO);
		for (unsigned int i = 0; i < 4; i++)
		{
			model = glm::mat4(1.0f);
			model = glm::translate(model, pointLightPositions[i]);
			model = glm::scale(model, glm::vec3(0.2f));
			lampShader.SetMat4("model", model);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		// +=================+
		// |   second pass   |
		// +=================+
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		screenShader.Use();
		glBindVertexArray(quadVAO);
		glDisable(GL_DEPTH_TEST);
		glBindTexture(GL_TEXTURE_2D, texColorBuffer);
		glDrawArrays(GL_TRIANGLES, 0, 6);

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

	// Cleanup Framebuffer
	glDeleteFramebuffers(1, &fbo);
	
	// Cleanup OpenGL buffers
	glDeleteVertexArrays(1, &lightVAO);
	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteBuffers(1, &VBO);

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