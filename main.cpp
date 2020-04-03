#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>
#include "Shader.h"
#include "Camera.h"
#include "Material.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xPos, double yPos);
void processInput(GLFWwindow* window);
unsigned int loadCubemap(std::vector<std::string> faces);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

#pragma region Camera Declare
//初始化摄像机
Camera myCamera(glm::vec3(0, 0, 3.0f), glm::radians(0.0f), glm::radians(180.0f), glm::vec3(0, 1.0f, 0));
#pragma endregion

#pragma region Input Declare
float lastX, lastY;
bool firstMouse = true;
#pragma endregion

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

	// 创建glfw窗口
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// 打开深度测试
	glEnable(GL_DEPTH_TEST);

	// 建立和编译着色器
	Shader skyboxShader("skybox.vs", "skybox.fs");

	// 设置顶点数据（和缓冲区）并配置顶点属性
	float skyboxVertices[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};

	// skybox VAO
	unsigned int skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	std::vector<std::string> faces
	{
		"skybox/right.jpg",
		"skybox/left.jpg",
		"skybox/top.jpg",
		"skybox/bottom.jpg",
		"skybox/front.jpg",
		"skybox/back.jpg"
	};
	unsigned int cubemapTexture = loadCubemap(faces);

	// 着色器配置
	skyboxShader.use();
	skyboxShader.setInt("skybox", 0);

#pragma region Prepare MVP matrices
	//空间变换代码
	glm::mat4 modelMat;
	//modelMat = glm::rotate(modelMat, glm::radians(-1.0f), glm::vec3(1.0f, 1.0, 0.5f));
	glm::mat4 viewMat;
	//viewMat = glm::translate(viewMat, glm::vec3(0, 0, -3.0f));
	//viewMat = myCamera.GetViewMatrix();
	glm::mat4 projMat;
	projMat = glm::perspective(glm::radians(45.0f), 800.0f / 800.0f, 0.1f, 100.0f);
#pragma endregion

	while (!glfwWindowShouldClose(window))
	{
		// 输入
		processInput(window);
		viewMat = myCamera.GetViewMatrix();

		// 渲染
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 绘制天空盒
		glDepthFunc(GL_LEQUAL);
		skyboxShader.use();
		viewMat = glm::mat4(glm::mat3(myCamera.GetViewMatrix()));
		skyboxShader.setMat4("view", viewMat);
		skyboxShader.setMat4("projection", projMat);
		// skybox cube
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS);

		// glfw：交换缓冲区和轮询IO事件（按下/释放键，鼠标移动等）
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// 清除资源
	glDeleteVertexArrays(1, &skyboxVAO);
	glDeleteBuffers(1, &skyboxVAO);
	glfwTerminate();
	return 0;
}

// 接收按键事件的函数
void processInput(GLFWwindow* window)
{
	float speed = 0.1f;//总体速度
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)//是否按下了返回键
		glfwSetWindowShouldClose(window, true);
	//前后移动
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		myCamera.speedZ = speed;
	else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		myCamera.speedZ = -speed;
	else
		myCamera.speedZ = 0;
	//左右移动
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		myCamera.speedX = -speed;
	else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		myCamera.speedX = speed;
	else
		myCamera.speedX = 0;
	//上下移动
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		myCamera.speedY = speed;
	else if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
		myCamera.speedY = -speed;
	else
		myCamera.speedY = 0;
}

// 接受鼠标事件的函数
void mouse_callback(GLFWwindow* window, double xPos, double yPos)
{
	if (firstMouse == true)
	{
		lastX = xPos;
		lastY = yPos;
		firstMouse = false;
	}
	float deltaX, deltaY;
	deltaX = xPos - lastX;
	deltaY = yPos - lastY;

	lastX = xPos;
	lastY = yPos;

	myCamera.ProcessMouseMovement(deltaX, deltaY);
}

// 窗口大小变化的函数
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

// 绑定天空盒贴图
unsigned int loadCubemap(std::vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrComponents;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrComponents, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}
