#include <iostream>
#include <vector>
#include <cassert>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
//SP
#include <glm/gtx/string_cast.hpp>

#include "util.hpp"
#include "mesh.hpp"



// Window handle.
void initGLFW();
GLFWwindow* window = nullptr;
int width = 800, height = 600;

// Shaders, location to uniform variables, and vertex array objects.
void initOpenGL();
void prepareScene();
GLuint VAO, VBO;
GLuint shader;
GLuint transformLocation;
Mesh* mesh;

// camera
glm::vec3 camCoords = glm::vec3(0.0, 0.0, 1.0);
bool camRot = false;
glm::vec2 camOrigin;
glm::vec2 mouseOrigin;

// View mode. When running the application, press 'M' key to switch mode.
const int VIEWMODE_TRIANGLE = 0;
const int VIEWMODE_OBJ = 1;
int viewMode = VIEWMODE_TRIANGLE;

// GLFW window callbacks to handle keyboard and mouse input.
void scrollCallback(GLFWwindow* w, double x, double y);
void keyCallback(GLFWwindow* w, int key, int sc, int action, int mode);
void mouseButtonCallback(GLFWwindow* w, int b, int action, int mode);
void cursorPosCallback(GLFWwindow* w, double xp, double yp);
void framebufferSizeCallback(GLFWwindow* w, int width, int height);

//glm::mat4 view;
int check_counter = 0;

int main() {
	std::cout << "Hello, OpenGL!" << std::endl;
	initGLFW();
	initOpenGL();
	prepareScene();

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		glClearColor(0.1, 0.2, 0.25, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Render pass
		glUseProgram(shader);
		glm::mat4 transform;
		glm::mat4 transform1;
		float aspect = (float)width / (float)height;
		glm::mat4 proj = glm::perspective(45.0f, aspect, 0.1f, 100.0f);
		//modifying the view (making it global)
		glm::mat4 view = glm::translate(glm::mat4(1.0f), { 0.0f, 0.0f, -camCoords.z });
		glm::mat4 rot = glm::rotate(glm::mat4(1.0f), glm::radians(camCoords.y), { 1.0f, 0.0f, 0.0f });
		rot = glm::rotate(rot, glm::radians(camCoords.x), { 0.0f, 1.0f, 0.0f });
		transform = proj * view * rot;

		if (viewMode == VIEWMODE_TRIANGLE) {
			glBindVertexArray(VAO);
			glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(transform));
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
		//need to change here
		else if (viewMode == VIEWMODE_OBJ) {
			std::cout << "Inside viewmode of obj type \n";
			auto meshBB = mesh->boundingBox();
			float bboxDiagLength = glm::length(meshBB.second - meshBB.first);
			glm::mat4 fixBB = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f / bboxDiagLength));
			fixBB = glm::translate(fixBB, -(meshBB.first + meshBB.second) / 2.0f);
			transform = transform * fixBB;
			//SP (start)
			glm::mat4 myIdentityMatrix = glm::mat4(1.0f);
			glm::mat4 myMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(1.05f, 0.0f, 0.0f));
			//std::cout << "\n final model matrix" << glm::to_string(fixBB);
			//std::cout << "\n final transform matrix" << glm::to_string(transform);
			//SP (end)
			//need to change the transform or transform location??
			if (check_counter == 0)
			{
				transform = transform * myMatrix;
				glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(transform));
				mesh->draw();
				transform1 = transform;
			}
			else
			{
				transform1 = transform1 * myMatrix;
				glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(transform1));
				mesh->draw();
			}
			check_counter++;

		}
		glBindVertexArray(0);
		glUseProgram(0);

		assert(glGetError() == GL_NO_ERROR);

		glfwSwapBuffers(window);
	}

	return 0;
}

void initGLFW() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	window = glfwCreateWindow(width, height, "OpenGL Demo", nullptr, nullptr);
	if (!window) {
		std::cerr << "Cannot create window";
		std::exit(1);
	}
	glfwMakeContextCurrent(window);

	glfwSetKeyCallback(window, keyCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetCursorPosCallback(window, cursorPosCallback);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	glfwSetScrollCallback(window, scrollCallback);
}

void initOpenGL() {
	assert(window);
	if (gladLoadGLLoader((GLADloadproc)(glfwGetProcAddress)) == 0) {
		std::cerr << "Failed to intialize OpenGL loader" << std::endl;
		std::exit(1);
	}
	assert(glGetError() == GL_NO_ERROR);
}

void prepareScene() {
	glEnable(GL_DEPTH_TEST);
	struct Vertex {
		glm::vec3 pos;
		glm::vec3 norm;
	};

	std::vector<Vertex> verts = {
		{{-0.433f, -0.25f, 0.0f}, {1.0, 0.0, 0.0}},
		{{ 0.433f, -0.25f, 0.0f}, {0.0, 1.0, 0.0}},
		{{ 0.0f,    0.5f,  0.0f}, {0.0, 0.0, 1.0}}
	};

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(verts[0]), verts.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, norm));

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Prepares the shader
	std::vector<GLuint> shaders;
	shaders.push_back(compileShader(GL_VERTEX_SHADER, "sh_v.glsl"));
	shaders.push_back(compileShader(GL_FRAGMENT_SHADER, "sh_f.glsl"));
	shader = linkProgram(shaders);
	transformLocation = glGetUniformLocation(shader, "xform");

	assert(glGetError() == GL_NO_ERROR);
}
// GLFW window callbacks
// --------------------------------------------------------------------

void scrollCallback(GLFWwindow* w, double x, double y) {
	float offset = (y > 0) ? 0.1f : -0.1f;
	camCoords.z = glm::clamp(camCoords.z + offset, 0.1f, 10.0f);
}

void keyCallback(GLFWwindow* w, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE) {
		glfwSetWindowShouldClose(w, true);
	}
	else if (key == GLFW_KEY_M && action == GLFW_RELEASE) {
		viewMode = (viewMode == VIEWMODE_TRIANGLE ? VIEWMODE_OBJ : VIEWMODE_TRIANGLE);
		if (viewMode == VIEWMODE_OBJ && mesh == NULL) {
			std::cout << "new model has loaded \n";
			mesh = new Mesh("models/cow.obj"); //initially cow.obj
			
		}
	}
}

void mouseButtonCallback(GLFWwindow* w, int button, int action, int mode) {
	//cout << "Inside the mouse button callback \n";
	//SP (not moving anything)
	int counter = 0;
	float delta = 1.0;

	/*
	while (counter < 50)
	{
		float radius = 10.0f;
		//std::cout << "glfwGetTime() = " << glfwGetTime() << "\n";
		float camX = sin(glfwGetTime()) * radius;
		float camZ = cos(glfwGetTime()) * radius;
		//glm::mat4 view;
		view = glm::lookAt(glm::vec3(camX, 0.0, camZ), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
		//std::cout << "current modified view " << glm::to_string(view) << "\n";
		std::cout << "\n camera position " << glm::to_string(camCoords);
		//camCoords = glm::vec3(0.0, 0.0, 1.0 + delta); //only z axis delta addition is affecting
		counter++;
	}	
	*/
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		// Activate rotation mode
		std::cout << "Left button has pressed \n";
		camRot = true;
		camOrigin = glm::vec2(camCoords);
		double xpos, ypos;
		glfwGetCursorPos(w, &xpos, &ypos);
		mouseOrigin = glm::vec2(xpos, ypos);
	} if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		camRot = false;
	}
}

void cursorPosCallback(GLFWwindow* w, double xp, double yp) {
	if (camRot) {
		float rotScale = std::fmin(width / 450.f, height / 270.f);
		glm::vec2 mouseDelta = glm::vec2(xp, yp) - mouseOrigin;
		glm::vec2 newAngle = camOrigin + mouseDelta / rotScale;
		newAngle.y = glm::clamp(newAngle.y, -90.0f, 90.0f);
		while (newAngle.x > 180.0f) newAngle.x -= 360.0f;
		while (newAngle.y < -180.0f) newAngle.y += 360.0f;
		if (glm::length(newAngle - glm::vec2(camCoords)) > std::numeric_limits<float>::epsilon()) {
			camCoords.x = newAngle.x;
			camCoords.y = newAngle.y;
		}
	}
}

void framebufferSizeCallback(GLFWwindow* w, int width, int height) {
	::width = width;
	::height = height;
	glViewport(0, 0, width, height);
}

