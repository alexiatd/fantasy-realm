#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <chrono>

#include <glm/glm.hpp> //core glm functionality
#include <glm/gtc/matrix_transform.hpp> //glm extension for generating common transformation matrices
#include <glm/gtc/matrix_inverse.hpp> //glm extension for computing inverse matrices
#include <glm/gtc/type_ptr.hpp> //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"
#include "Skybox.hpp"

#include <iostream>

std::chrono::steady_clock::time_point startTime;


// window
gps::Window myWindow;

// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;
glm::mat4 lightRotation;


// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;

// shader uniform locations
GLint modelLoc;
GLint viewLoc;
GLint projectionLoc;
GLint normalMatrixLoc;
GLint lightDirLoc;
GLint lightColorLoc;

//fog uniform Location
GLint fogDensityLoc;

//spotlight uniform Location
GLint spotlightLoc;

int width = 1920;
int height = 1080;

// camera
gps::Camera myCamera(
	glm::vec3(43.7f, 3.5f, -53.0f),
	glm::vec3(0.0f, 10.0f, 30.0f),
	glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 0.2f;

GLboolean pressedKeys[1024];

GLfloat angle = 0.0f;

//light Angle 
GLfloat lightAngle;
glm::mat4 lightRot;
glm::mat4 modelLight;

glm::vec3 luminaShop = glm::vec3(3.62f, -4.6092f, 96.5292f);
int spot = 0;

gps::Model3D scene;
gps::Model3D bunny;
gps::Model3D cat;
gps::Model3D duck;
gps::Model3D guard;
gps::Model3D vortex;
gps::Model3D cow;

glm::mat4 modelObjBunny;
glm::mat4 modelObjCat;
glm::mat4 modelObjVortex;
glm::mat4 modelObjCow;
glm::mat4 modelObjGuard;
glm::mat4 modelObjDuck;


// skybox
gps::SkyBox mySkyBox;
gps::Shader skyboxShader;

glm::vec3 cameraPosition;
GLint cameraPositionLoc;

glm::float32 fog_density = 0.00f;

// shaders
gps::Shader myBasicShader;

// shaders
gps::Shader lightShader;
//shadow
gps::Shader depthMapShader;


//Shadow
GLuint shadowMapFBO;
GLuint depthMapTexture;
const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;


float val = 0.0f;
int firstpress = 0;
int fog = 0;
int front = 1, back = 0;


GLenum glCheckError_(const char* file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR) {
		std::string error;
		switch (errorCode) {
		case GL_INVALID_ENUM:
			error = "INVALID_ENUM";
			break;
		case GL_INVALID_VALUE:
			error = "INVALID_VALUE";
			break;
		case GL_INVALID_OPERATION:
			error = "INVALID_OPERATION";
			break;
		case GL_STACK_OVERFLOW:
			error = "STACK_OVERFLOW";
			break;
		case GL_STACK_UNDERFLOW:
			error = "STACK_UNDERFLOW";
			break;
		case GL_OUT_OF_MEMORY:
			error = "OUT_OF_MEMORY";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			error = "INVALID_FRAMEBUFFER_OPERATION";
			break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {

	WindowDimensions win;

	fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);

	win.width = width;
	win.height = height;


	myWindow.setWindowDimensions(win);
}

float valx = -25.0f;
float valy = 0.0f;

bool animation = false;
bool guard_animation = false;
bool cat_animation = false;

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_X && action == GLFW_PRESS) {
		animation = !animation;
	}

	if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
		cat_animation = !cat_animation;
	}

	if (key == GLFW_KEY_C && action == GLFW_PRESS) {
		guard_animation = !guard_animation;
	}

	if (key == GLFW_KEY_F && action == GLFW_PRESS) {
		if (fog == 1) fog = 0;
		else {
			fog = 1;
		}
		myBasicShader.useShaderProgram();
		glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "fog"), fog);

	}

	if (key == GLFW_KEY_P && action == GLFW_PRESS) {
		if (spot == 1) spot = 0;
		else {
			spot = 1;
		}
		myBasicShader.useShaderProgram();
		glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "spot"), spot);

	}

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	if (key >= 0 && key < 1024) {
		if (action == GLFW_PRESS) {
			pressedKeys[key] = true;
		}
		else if (action == GLFW_RELEASE) {
			pressedKeys[key] = false;
		}
	}
}

float sensitivity = 0.5f;

bool firstMouse = true;
float lastX = 400, lastY = 300;
float pitch = 0.0f, yaw = -90.0f;

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.05f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	myCamera.rotate(pitch, yaw);
	myCamera.getViewMatrix();  // This line is not necessary and can be removed

}


float rotateSpeed = 15.5f;


void processMovement() {
	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);

		view = myCamera.getViewMatrix();
		myBasicShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);

		view = myCamera.getViewMatrix();
		myBasicShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);

		modelObjDuck = glm::rotate(modelObjDuck, glm::radians(rotateSpeed), glm::vec3(1.0, 0.0, 0.0));

		view = myCamera.getViewMatrix();
		myBasicShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);

		modelObjDuck = glm::rotate(modelObjDuck, glm::radians(-rotateSpeed), glm::vec3(1.0, 0.0, 0.0));

		view = myCamera.getViewMatrix();
		myBasicShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	if (pressedKeys[GLFW_KEY_Q]) {
		angle -= 1.0f;

		model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));

		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	if (pressedKeys[GLFW_KEY_E]) {
		angle += 1.0f;

		model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));

		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	if (pressedKeys[GLFW_KEY_H]) {
		if (fog == 1) {
			fog_density -= 0.001f;
		}
		else {
			fog_density = 0.0f;
		}

		myBasicShader.useShaderProgram();
		glUniform1f(fogDensityLoc, fog_density);

	}

	if (pressedKeys[GLFW_KEY_G]) {
		if (fog == 1) {
			fog_density += 0.001f;
		}
		else {
		}
		myBasicShader.useShaderProgram();
		glUniform1f(fogDensityLoc, fog_density);
	}

	if (pressedKeys[GLFW_KEY_B]) {

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	if (pressedKeys[GLFW_KEY_N])
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	if (pressedKeys[GLFW_KEY_M]) {

		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	}

	if (pressedKeys[GLFW_KEY_UP]) {

		valy = valy - 0.2f;
	}

	if (pressedKeys[GLFW_KEY_DOWN]) {

		valy = valy + 0.2f;
	}

	if (pressedKeys[GLFW_KEY_LEFT]) {

		valx = valx - 0.2f;
	}

	if (pressedKeys[GLFW_KEY_RIGHT]) {

		valx = valx + 0.2f;
	}


}

void initOpenGLWindow() {
	myWindow.Create(1920, 1000, "Boopsie Core");
}

void setWindowCallbacks() {
	glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
	glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
	glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
	glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void initOpenGLState() {
	startTime = std::chrono::steady_clock::now();
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
	glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
	glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	//glEnable(GL_CULL_FACE); // cull face
	//glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
	glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
}

void initModels() {
	scene.LoadModel("models/scene/forest1.obj");
	bunny.LoadModel("models/bunny/bunnyhead.obj");
	vortex.LoadModel("models/vortex/vortex1.obj");
	cow.LoadModel("models/cow/cow.obj");
	cat.LoadModel("models/wiz/wizcat.obj");
	guard.LoadModel("models/guard/guard.obj");
	duck.LoadModel("models/duck/duck.obj");

}

void initShaders() {
	myBasicShader.loadShader(
		"shaders/basic.vert",
		"shaders/basic.frag");

	skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
	skyboxShader.useShaderProgram();

	depthMapShader.loadShader("shaders/depthMapShader.vert", "shaders/depthMapShader.frag");


}

void initUniforms() {
	myBasicShader.useShaderProgram();

	// create model matrix for teapot
	model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
	modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");

	// get view matrix for current camera
	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
	// send view matrix to shader
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	// compute normal matrix for teapot
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

	// create projection matrix
	projection = glm::perspective(glm::radians(45.0f),
		(float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
		0.1f, 2000.0f);
	projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
	// send projection matrix to shader
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(0.0f, 50.0f, 1.0f);
	lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");

	lightRot = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	// send light dir to shader
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
	// send light color to shader
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	//set fog density
	fogDensityLoc = glGetUniformLocation(myBasicShader.shaderProgram, "fogDensity");

	cameraPosition = myCamera.getPosition();
	cameraPositionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "cameraPos");
	glUniform3fv(cameraPositionLoc, 1, glm::value_ptr(cameraPosition));

}

float getCurrentTime() {
	// Get the current time
	auto currentTime = std::chrono::steady_clock::now();

	// Calculate the duration since the start time
	std::chrono::duration<float> duration = currentTime - startTime;
	return duration.count();
}


void renderBunny(gps::Shader shader)
{
	shader.useShaderProgram();

	modelObjBunny = glm::mat4(1.0f);

	glm::vec3 bunnyPosition = glm::vec3(2.00751f, -3.03534f, 98.4374f);

	float rotationSpeed = 1.5f;
	float currentTime = getCurrentTime();

	float bunnyangle = 15.0f * glm::sin(rotationSpeed * currentTime);


	modelObjBunny = glm::rotate(modelObjBunny, glm::radians(angle), glm::vec3(0.0, 1.0, 0.0));

	modelObjBunny = glm::translate(modelObjBunny, -bunnyPosition);
	modelObjBunny = glm::rotate(modelObjBunny, glm::radians(bunnyangle), glm::vec3(1.0, 0.0, 0.0));
	modelObjBunny = glm::translate(modelObjBunny, bunnyPosition);

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelObjBunny));

	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::inverseTranspose(view * modelObjBunny))));

	bunny.Draw(shader);
}

void renderDuck(gps::Shader shader)
{
	shader.useShaderProgram();

	modelObjDuck = glm::mat4(1.0f);

	glm::vec3 duckPosition = glm::vec3(0.0f, 0.0f, 0.0f);

	modelObjDuck = glm::rotate(modelObjDuck, glm::radians(angle), glm::vec3(0.0, 1.0, 0.0));

	modelObjDuck = glm::translate(modelObjDuck, glm::vec3(valx, -0.9, valy));

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelObjDuck));

	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::inverseTranspose(view * modelObjDuck))));


	duck.Draw(shader);
}

void renderGuard(gps::Shader shader)
{
	shader.useShaderProgram();

	// Reset model matrix
	modelObjGuard = glm::mat4(1.0f);

	// Set the position where you want the guard to be initially
	glm::vec3 guardPosition = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 guardPositionF = glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 guardPositionFinal = glm::vec3(0.0f, 0.0f, 3.0f);

	// Apply translation to the model matrix to position the guard
	modelObjGuard = glm::translate(modelObjGuard, guardPosition);
	modelObjGuard = glm::rotate(modelObjGuard, glm::radians(angle), glm::vec3(0.0, 1.0, 0.0));
/*
	float forward = 0.0f;
	float forward1 = 0.0f;

	if (guard_animation) {

		float guardangle = 90.0f;

		modelObjGuard = glm::translate(modelObjGuard, guardPosition);
		modelObjGuard = glm::rotate(modelObjGuard, glm::radians(guardangle), glm::vec3(0.0, 1.0, 0.0));
		modelObjGuard = glm::translate(modelObjGuard, -guardPosition);

		if (forward < 1.5f) {
			modelObjGuard = glm::translate(modelObjGuard, glm::vec3(forward, 0.0f, 0.0f));
			forward += 0.1f;

		}
		//modelObjGuard = glm::translate(modelObjGuard, guardPositionFinal);

		modelObjGuard = glm::translate(modelObjGuard, guardPositionF);
		modelObjGuard = glm::rotate(modelObjGuard, glm::radians(guardangle), glm::vec3(0.0, 1.0, 0.0));
		modelObjGuard = glm::translate(modelObjGuard, -guardPositionF);

		if (forward1 < 1.5f) {
			modelObjGuard = glm::translate(modelObjGuard, glm::vec3(forward1, 0.0f, 0.0f));
			forward1 += 0.1f;
		}

	}
	else {
		modelObjGuard = glm::translate(modelObjGuard, guardPosition);

	}
	*/
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelObjGuard));

	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::inverseTranspose(view * modelObjGuard))));

	guard.Draw(shader);
}

void renderWizardMovement(gps::Shader shader) {
	shader.useShaderProgram();

	glm::vec3 catPosition = glm::vec3(1.2169f, 1.14307f, 1.0f);
	glm::vec3 catPositionUp = glm::vec3(36.2169f, -3.02937f, 56.2775f);

	// Calculate vertical movement using sine function
	float verticalMovement = 0.5f * glm::sin(0.5f * getCurrentTime());

	// Reset model matrix
	modelObjCat = glm::mat4(1.0f);
	modelObjCat = glm::rotate(modelObjCat, glm::radians(angle), glm::vec3(0.0, 1.0, 0.0));
	modelObjCat = glm::translate(modelObjCat, catPosition);


	if (cat_animation == true) {

		modelObjCat = glm::translate(modelObjCat, glm::vec3(0.0f, 0.0f, 0.0f));
		glm::vec3 catCurrentPosition = catPosition + glm::vec3(0.0f, verticalMovement, 0.0f);
		modelObjCat = glm::translate(modelObjCat, catCurrentPosition);
	}
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelObjCat));

	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::inverseTranspose(view * modelObjCat))));

	cat.Draw(shader);
}

void renderVortex(gps::Shader shader) {
	glm::mat4 start(1.0f);
	shader.useShaderProgram();
	modelObjVortex = glm::mat4(1.0f);

	glm::vec3 vortexPosition = glm::vec3(43.6297f, 4.62019f, 50.2918f);
	modelObjVortex = glm::rotate(start, glm::radians(angle), glm::vec3(0.0, 1.0, 0.0));

	// Get the current time
	float currentTime = glfwGetTime();

	// Calculate the rotation angle based on time
	float rotationSpeed = 30.0f;
	float vortexangle = glm::radians(rotationSpeed * currentTime);

	modelObjVortex = glm::translate(modelObjVortex, vortexPosition);
	modelObjVortex = glm::rotate(modelObjVortex, vortexangle, glm::vec3(0.0f, 0.0f, 1.0f));
	modelObjVortex = glm::translate(modelObjVortex, -vortexPosition);

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelObjVortex));

	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::inverseTranspose(view * modelObjVortex))));

	vortex.Draw(shader);

}


void renderCowMovement(gps::Shader shader) {
	glm::mat4 start(1.0f);
	shader.useShaderProgram();

	//modelObjCow = glm::mat4(1.0f);

	glm::vec3 cowPosition = glm::vec3(-1.787685f, 0.0f, 1.33f);

	modelObjCow = glm::rotate(start, glm::radians(angle), glm::vec3(0.0, 1.0, 0.0));

	// modelObjCow = glm::rotate(modelObjCow, angle, glm::vec3(0.0f, 1.0f, 0.0f));
	modelObjCow = glm::translate(modelObjCow, cowPosition);

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelObjCow));

	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::inverseTranspose(modelObjCow))));

	cow.Draw(shader);
}


void renderScene1(gps::Shader shader) {

	shader.useShaderProgram();

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	scene.Draw(shader);
}

void renderWithShadow(gps::Model3D* object, int  obj)
{
	depthMapShader.useShaderProgram();
	glm::mat4 start(1.0f);

	if (obj == 0) // scene
		glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	if (obj == 1)  // duck
	{
		modelObjDuck = glm::translate(modelObjDuck, glm::vec3(0.0f, 0.0f, 0.0f));

		glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelObjDuck));
	}
	else if (obj == 2) // bunny
	{
		modelObjBunny = glm::translate(modelObjBunny, glm::vec3(0.0f, 0.0f, 0.0f));

		glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelObjBunny));
	}
	else if (obj == 3) // cow
	{
		modelObjCow = glm::translate(modelObjCow, glm::vec3(0.0f, 0.0f, 0.0f));

		glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelObjCow));
	}
	else if (obj == 4) // guard
	{
		modelObjGuard = glm::translate(modelObjGuard, glm::vec3(0.0f, 0.0f, 0.0f));

		glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelObjGuard));
	}
	else if (obj == 5) // cat
	{
		modelObjCat = glm::translate(modelObjCat, glm::vec3(0.0f, 0.0f, 0.0f));

		glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelObjCat));
	}

	(*object).Draw(depthMapShader);

}

glm::mat4 computeLightSpaceTrMatrix() {

	glm::mat4 lightView = glm::lookAt(glm::mat3(lightRotation) * lightDir, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	const GLfloat near_plane = 0.1f, far_plane = 150.0f;
	glm::mat4 lightProjection = glm::ortho(-300.0f, 300.0f, -300.0f, 300.0f, near_plane, far_plane);
	glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;

	return lightSpaceTrMatrix;

}
void renderScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	depthMapShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);


	renderWithShadow(&scene, 0);
	renderWithShadow(&duck, 1);
	renderWithShadow(&bunny, 2);
	renderWithShadow(&cow, 3);
	renderWithShadow(&guard, 4);
	renderWithShadow(&cat, 5);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// final scene rendering pass 

	glViewport(0, 0, (float)myWindow.getWindowDimensions().width, (float)myWindow.getWindowDimensions().height);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	myBasicShader.useShaderProgram();

	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

	//bind the shadow map
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "shadowMap"), 3);

	glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));


	renderBunny(myBasicShader);
	renderCowMovement(myBasicShader);
	renderWizardMovement(myBasicShader);
	renderGuard(myBasicShader);
	renderDuck(myBasicShader);
	renderScene1(myBasicShader);

	renderVortex(myBasicShader);

	mySkyBox.Draw(skyboxShader, view, projection);

}

void initFBO() {
	//Create the FBO, the depth texture and attach the depth texture to the FBO - taken from laboratory 
	//generate FBO ID
	glGenFramebuffers(1, &shadowMapFBO);

	//create depth texture for FBO
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	//attach texture to FBO
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture,
		0);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void initSkybox() {
	std::vector<const GLchar*> faces;
	faces.push_back("skybox/right.png");
	faces.push_back("skybox/left.png");
	faces.push_back("skybox/top.png");
	faces.push_back("skybox/bottom.png");
	faces.push_back("skybox/back.png");
	faces.push_back("skybox/front.png");
	mySkyBox.Load(faces);

}

void cleanup() {
	myWindow.Delete();
	//cleanup code for your own data
}



void updateViewAndNormalMatrix() {
	view = myCamera.getViewMatrix();
	myBasicShader.useShaderProgram();
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
}

int move = 0;
GLfloat cameraSpeed1 = 0.1f;

void presentation() {
	if (animation) {
		if (move < 300) {
			// Move forward
			myCamera.move(gps::MOVE_FORWARD, cameraSpeed1);
			move++;

			// Update view and normal matrix
			updateViewAndNormalMatrix();

		}
		else if (move == 300) {
			// Rotate
			yaw += 0.2;
			myCamera.rotate(pitch, yaw);

			// Update view and normal matrix
			updateViewAndNormalMatrix();
		}
	}
}


int main(int argc, const char* argv[]) {

	try {
		initOpenGLWindow();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	initOpenGLState();
	initModels();
	initShaders();
	initUniforms();
	initFBO();
	setWindowCallbacks();
	initSkybox();


	glCheckError();
	// application loop
	while (!glfwWindowShouldClose(myWindow.getWindow())) {
		processMovement();

		presentation();

		renderScene();

		glfwPollEvents();
		glfwSwapBuffers(myWindow.getWindow());

		glCheckError();
	}

	cleanup();

	return EXIT_SUCCESS;
}
